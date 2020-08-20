# Copyright (C) 2011 Nippon Telegraph and Telephone Corporation.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller.handler import CONFIG_DISPATCHER, MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.lib.packet import ipv4
from ryu.lib.packet import tcp
from ryu.lib.packet import ether_types


class SimpleSwitch13(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]
    
    def __init__(self, *args, **kwargs):
        super(SimpleSwitch13, self).__init__(*args, **kwargs)
        self.mac_to_port = {}
	self.count_packet_in=0

    @set_ev_cls(ofp_event.EventOFPSwitchFeatures, CONFIG_DISPATCHER)
    def switch_features_handler(self, ev):
        datapath = ev.msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        # install table-miss flow entry
        #
        # We specify NO BUFFER to max_len of the output action due to
        # OVS bug. At this moment, if we specify a lesser number, e.g.,
        # 128, OVS will send Packet-In with invalid buffer_id and
        # truncated packet data. In that case, we cannot output packets
        # correctly.  The bug has been fixed in OVS v2.1.0.
        match = parser.OFPMatch()
        actions = [parser.OFPActionOutput(ofproto.OFPP_CONTROLLER,
                                          ofproto.OFPCML_NO_BUFFER)]
        self.add_flow(datapath, 0, match, actions)

    def add_flow(self, datapath, priority, match, actions, buffer_id=None):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        #print("ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ")

        inst = [parser.OFPInstructionActions(ofproto.OFPIT_APPLY_ACTIONS,
                                             actions)]
        if buffer_id:
            #print("YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY")
            mod = parser.OFPFlowMod(datapath=datapath, buffer_id=buffer_id,
                                    priority=priority, match=match,
                                    instructions=inst)
        else:
            #print("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")
            mod = parser.OFPFlowMod(datapath=datapath, priority=priority,
                                    match=match, instructions=inst)
        #print(mod)
        datapath.send_msg(mod)

    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        # If you hit this you might want to increase
        # the "miss_send_length" of your switch
        #print("test")
	self.count_packet_in=self.count_packet_in+1;
	print("count:"+ str(self.count_packet_in))
        if ev.msg.msg_len < ev.msg.total_len:
            self.logger.debug("packet truncated: only %s of %s bytes",
                              ev.msg.msg_len, ev.msg.total_len)
        msg = ev.msg
        datapath = msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        in_port = msg.match['in_port']

        pkt = packet.Packet(msg.data)
        eth = pkt.get_protocols(ethernet.ethernet)[0]
        if(len(pkt.get_protocols(ipv4.ipv4)) != 0):
            ip = pkt.get_protocols(ipv4.ipv4)[0] ###################### Myself
            ip_src = ip.src ####################### Myself
            ip_dst = ip.dst ####################### Myself
            #print(ip)
            ip_proto = ip.proto
            if(len(pkt.get_protocols(tcp.tcp)) != 0):
                tcp_mod = pkt.get_protocols(tcp.tcp)[0]
                tcp_src = tcp_mod.src_port
                tcp_dst = tcp_mod.dst_port
                #print(tcp_mod)
                #ip_proto = ip.proto
        #print(pkt)
        if eth.ethertype == ether_types.ETH_TYPE_LLDP:
            # ignore lldp packet
            #print("AAAAAAAAAAAAAAAAAAA")
            return
        eth_type = eth.ethertype
        dst = eth.dst
        src = eth.src
        #ip_src = ip.src ####################### Myself
        #ip_dst = ip.dst ####################### Myself

        dpid = datapath.id
        self.mac_to_port.setdefault(dpid, {})

        #print("BBBBBBBBBBBBBBBBBBBBBBBB")
        #self.logger.info("packet in %s %s %s %s", dpid, src, dst, in_port)

        # learn a mac address to avoid FLOOD next time.
        self.mac_to_port[dpid][src] = in_port

        if dst in self.mac_to_port[dpid]:
            out_port = self.mac_to_port[dpid][dst]
            #print("CCCCCCCCCCCCCCCCCCCCCCC")
        else:
            out_port = ofproto.OFPP_FLOOD
            #print("DDDDDDDDDDDDDDDDDDDDDDD")

        actions = [parser.OFPActionOutput(out_port)]

        # install a flow to avoid packet_in next time
        if out_port != ofproto.OFPP_FLOOD:
            #print("EEEEEEEEEEEEEEEEEEEEEE")
            if (len(pkt.get_protocols(ipv4.ipv4)) != 0):
                #print("FFFFFFFFFFFFFFFFFFFFFFFFF")
                if( len(pkt.get_protocols(tcp.tcp))  == 0):
                    match = parser.OFPMatch(in_port=in_port,
                                            eth_dst=dst,
                                            eth_src=src,
                                            eth_type=eth_type,
                                            ipv4_src=ip_src,
                                            ipv4_dst=ip_dst,
                                            ip_proto=ip_proto)
                else: 
                    match = parser.OFPMatch(in_port=in_port,
                                            eth_dst=dst,
                                            eth_src=src,
                                            eth_type=eth_type,
                                            ipv4_src=ip_src,
                                            ipv4_dst=ip_dst,
                                            ip_proto=ip_proto,
                                            tcp_src=tcp_src,
                                            tcp_dst=tcp_dst)
                # verify if we have a valid buffer_id, if yes avoid to send both
                # flow_mod & packet_out
                if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                    #print("GGGGGGGGGGGGGGGGGGGGG")
                    self.add_flow(datapath, 1, match, actions, msg.buffer_id)
                    #return
                else:
                    #print("HHHHHHHHHHHHHHHHHHHHHH")
                    self.add_flow(datapath, 1, match, actions)

        data = None
        if msg.buffer_id == ofproto.OFP_NO_BUFFER:
            #print("IIIIIIIIIIIIIIIIIIIIIIIIIIII")
            data = msg.data
        #print("KKKKKKKKKKKKKKKKKKKKKKKKKKK")
        out = parser.OFPPacketOut(datapath=datapath, buffer_id=msg.buffer_id,
                                  in_port=in_port, actions=actions, data=data)
        #print("LLLLLLLLLLLLLLLLLLLLLLLLLLLL")
        #print(out)
        #print("\n")
        datapath.send_msg(out)

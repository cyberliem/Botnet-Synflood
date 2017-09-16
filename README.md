# Botnet-Synflood
A set of control message to execute distributed TCP syn flood using C Socket program. 
The program is designed and built to follow the Botnet which is distributed over three layers. The layers are controller, handler and agents.
There are 2 main types of botnet infrastructures, which are centralized and decentralized. In this documentation, the centralized architecture will be used. Detail explanations of different types of botnet technologies can be seen under Botnet Topologies. Even though there are different topologies under centralized, in our program, the hierarchical topology will be used. This topology can be seen in the following figure as follows. 

Hierarchical Topology
	This 3 layer hierarchical topology consists of Botmaster (Controller), Command and Control Server (Handler) and Attacking Bots (Agents). In depth detailed explanation for this topology can be seen under Hierarchical Topology in the upcoming chapters. 
  
Download and compile it: controller is from your main machine. Handler to your intermediate machines and inject Agents on zombie host.
From controller:
Add Agent to Handler: add a zombie host IP to a intermediate handler

        	
          ./ctrl 1 <Handler IP> <Agent IP>
          
          
Interorate Handler : check if the handlers are online.

        	
          ./ctrl 2 <Handler List File Name>
                    
Validate Agents : check if the agents from each handler are online. 
        	
          ./ctrl 3 <Handler List File Name>
          

Count Agents : return total number of agent. 


          ./ctrl 4 <Handler List File Name>
          
Attack: executing the attack on a host.
        	

          ./ctrl 5 <Handler List File Name> <Victim IP> [number of package]

          
A successful attack can be captured by wireshark for analysis purpose, as in pcap file.

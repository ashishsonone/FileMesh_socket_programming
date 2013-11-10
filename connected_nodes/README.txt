CS378 Socket Programming Lab: Distributed File System

Submitted By:
	Ashish Sonone (110050022)
	Ayush Kumar (110050038)
	Himanshu Roy (110050019)

---------------------------------------------------------------------------------------------------

List of relavant Files:
	node.cpp : The node program implementing the distributed file system(DFS).

	client.cpp : User or client program which makes a store or get request to the DFS.

	header.h : contain structs and utility functions used by node.cpp and client.cpp.

	FileMesh.cfg : the configuration file read by node.cpp and client.cpp to get (nodeid)->(address) mapping
		It must be in the same folder as the executable.

	exampleFileMesh.cfg : an example configuration file showing how "FileMesh.cfg" should look like. 

---------------------------------------------------------------------------------------------------

Compilation Instructions:
	Go to the directory containing node.cpp, client.cpp & header.h.
	
	node.cpp : (node program)
		g++ node.cpp  -o node.out -lpthread

	client.cpp : (client program)
		g++ client.cpp -o client.out

---------------------------------------------------------------------------------------------------

Configuration Files:
	Program requires a config file "FileMesh.cfg" to pass information about all nodes that are part of the mesh.
	This file must reside in the same folder as the executable(node.out, client.out)
	It must contain entries(lines) in following format: 

		IPaddr0:port0 folder0
		IPaddr1:port1 folder1
		.
		.
		and so on

	An example configuration file named "exampleFileMesh.cfg" is given in the same folder.

---------------------------------------------------------------------------------------------------

Running instructions:
	Node program: (node.out)
		./node.out  <node id>

		<node id> is the id of the node in the mesh. Varying from 0 to (n-1) given "n" nodes in the mesh.

	~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

	Client program: (client.out)
		./client.out  [<interface-name>]

		<interface-name> is the name of the interface (default "eth0" when this command line argument not given).
			This is needed to get the ip address of the client.
		So you could either run
		./client.out				---> interface name would be assumed to be "eth0"
		OR
		./client.out eth1			---> interface name will be taken as "eth1"

	Following are example runs of the client program on terminal for store and get request respectively: 

	<For Store Request> Enter nodeid, give option 0(for store), then specify 
						the filepath when prompted and you're done
	******************
		Enter first node id(to be contacted first) : 1
		Enter option[0 to Store, 1 to Get] :  0
		Enter file path : home/ashish/Desktop/client.cpp


	<For Get Request>	Enter nodeid, give option 1(for get), give folder location 
						where to save and then the md5sum of the file requested
	*****************
		Enter first node id(to be contacted first) : 2
		Enter option[0 to Store, 1 to Get] : 1
		Enter folder where to save : /home/ashish/Desktop
		Enter md5sum of file : 75351448765610149158c8f4eb14816b

---------------------------------------------------------------------------------------------------
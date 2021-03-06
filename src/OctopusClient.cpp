//  ----------------------------------
//  Holograms -- Copyright © 2016, Brown University, Providence, RI.
//  
//  All Rights Reserved
//   
//  Use of the software is provided under the terms of the GNU General Public License version 3 
//  as published by the Free Software Foundation at http://www.gnu.org/licenses/gpl-3.0.html, provided 
//  that this copyright notice appear in all copies and that the name of Brown University not be used in 
//  advertising or publicity pertaining to the use or distribution of the software without specific written 
//  prior permission from Brown University.
//  
//  BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE WHICH IS 
//  PROVIDED “AS IS”, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
//  FOR ANY PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR ANY 
//  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR FOR ANY DAMAGES WHATSOEVER RESULTING 
//  FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
//  OTHER TORTIOUS ACTION, OR ANY OTHER LEGAL THEORY, ARISING OUT OF OR IN CONNECTION 
//  WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
//  ----------------------------------
//  
///\file OctopusClient.cpp
///\author Benjamin Knorlein
///\date 11/13/2016

#include "OctopusClient.h"

#include <iostream>
#include <assert.h>

OctopusClient::OctopusClient(const std::string &serverIP, const std::string &serverPort) : m_mode(-1)
{
	m_sock = new Socket(serverIP, serverPort);


	// Send the API version to the Octopus software.  This is supposed to 
	// put the software into the special 'receive information through
	// back door' mode, and should more or less disable the GUI.
	
	std::cerr << "Set Api Version" << std::endl;
	m_sock->sendMessage("SET_API_VERSION 2\n");

	//clear pipe;
	char buffer[1024];

	std::cerr << "Empty socket" << std::endl;
	int n;
	while (((n = recv(*m_sock->getSocketFD(), buffer, sizeof(buffer), 0))>0))
	{
		if (n <= 0)
		{
			break;
		}
	}
	std::cerr << "Octopus connected" << std::endl;
}

OctopusClient::~OctopusClient()
{
	delete m_sock;
}

void OctopusClient::getPhaseImage(int depth, float* data)
{
	setOutputMode(2);
	receiveImageData(depth, data);
}
void OctopusClient::getIntensityImage(int depth, float* data)
{
	setOutputMode(0);
	receiveImageData(depth, data);
}
void OctopusClient::getAmplitudeImage(int depth, float* data)
{
	setOutputMode(1);
	receiveImageData(depth, data);
}

bool OctopusClient::setSourceHologram(std::string folder, std::string filename, std::string background)
{
	m_background = background;
	m_filename = filename;
	m_folder = folder;
	if (m_folder.back() != '/')m_folder.append("/");

	std::string backgroundString = "";
	if (m_background != "")
	{
		backgroundString = "*" + m_folder + m_background;
	}
	
	std::cerr << "send message" << std::endl;
	m_sock->sendMessage("RECONSTRUCT_HOLOGRAMS " + m_folder + m_filename + backgroundString + "\n0\n");
	std::cerr << "Wait for receive" << std::endl;
	//Sleep(1000);
	m_sock->receiveMessage(&m_dummyBuffer[0], 26);
	std::cerr << "Received" << std::endl;
	std::string reply = std::string(m_dummyBuffer, 26);
	if (reply != "RECONSTRUCT_HOLOGRAMS 0\n0\n" && reply != "RECONSTRUCT_HOLOGRAMS 1\n0\n" && reply != "RECONSTRUCT_HOLOGRAMS 2\n0\n") {
		std::cout << "REPLY:" << reply << ", expect:" << "RECONSTRUCT_HOLOGRAMS 0\n0\n or RECONSTRUCT_HOLOGRAMS 1\n0\n or RECONSTRUCT_HOLOGRAMS 2\n0\n" << std::endl;
		assert(reply == "RECONSTRUCT_HOLOGRAMS 0\n0\n" || reply == "RECONSTRUCT_HOLOGRAMS 1\n0\n" || reply == "RECONSTRUCT_HOLOGRAMS 2\n0\n");
	}

	//if (reply != "RECONSTRUCT_HOLOGRAMS 1\n0\n") return false;
#ifdef DEBUG
	std::cout << reply << std::endl;
#endif
	Sleep(500);
	return true;
}

void OctopusClient::receiveImageData(int depth, float* data)
{
	int datasize = 4 * 2048 * 2048;

	std::string message = "STREAM_RECONSTRUCTION " + std::to_string(depth) + "\n0\n";
#ifdef DEBUG
	std::cout << "Send" << message << std::endl;
#endif
	m_sock->sendMessage(message);

	//receive data
	std::string expectedReply = "STREAM_RECONSTRUCTION 2048 2048 " + m_background + " " + m_filename + " " + std::to_string(depth) + " " + std::to_string(m_mode) + "\n" + std::to_string(datasize) + "\n";

	std::string expectedReplyCutoff = "TREAM_RECONSTRUCTION 2048 2048 " + m_background + " " + m_filename + " " + std::to_string(depth) + " " + std::to_string(m_mode) + "\n" + std::to_string(datasize) + "\n";

	//Sleep(500);
	m_sock->receiveMessage(m_dummyBuffer, expectedReply.size());
#ifdef DEBUG
	std::cout << "dummyBuffer:" << m_dummyBuffer << std::endl;
#endif
	std::string reply = std::string(m_dummyBuffer, expectedReply.size());
        std::string replyCutoff = std::string(m_dummyBuffer, expectedReplyCutoff.size());

	if (reply != expectedReply && replyCutoff != expectedReplyCutoff) {
		std::cout << "REPLY:" << reply << ":vs XPCT:" << expectedReply << std::endl;
	// compare and send request again if different
	assert(reply == expectedReply);
	}
#ifdef DEBUG
	std::cout << "Receive " << reply << std::endl;
#endif
	char * dataPtr = (char *)&data[0];
	datasize = m_sock->receiveMessage(dataPtr, datasize);
	//int * ptr_org = (int*)&data[0];
	//int * ptr_dest = (int*)&data[0];

	//for (int i = 0; i < datasize / 4; i++, ptr_org++, ptr_dest++)
	//{
	//	*ptr_dest = (*ptr_org << 24) |
	//		((*ptr_org << 8) & 0x00ff0000) |
	//		((*ptr_org >> 8) & 0x0000ff00) |
	//		((*ptr_org >> 24) & 0x000000ff);
	//}
}

bool OctopusClient::setOutputMode(int mode)
{
	if (m_mode != mode){
		m_mode = mode;
		std::string message = "OUTPUT_MODE " + std::to_string(m_mode) + "\n0\n";
		m_sock->sendMessage(message);
		m_sock->receiveMessage(&m_dummyBuffer[0], message.size());
		std::string reply = std::string(m_dummyBuffer, message.size());
#ifdef DEBUG
		std::cout << reply << std::endl;
#endif
		assert(reply == message);
		if (reply != message) return false;
		Sleep(500);
	}

	return true;
}

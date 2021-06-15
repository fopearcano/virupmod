/*
    Copyright (C) 2020 Florian Cabot <florian.cabot@hotmail.fr>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "NetworkManager.hpp"

NetworkManager::NetworkManager(AbstractState* networkedState)
    : networkedState(networkedState)
{
	if(networkedState == nullptr)
	{
		return;
	}
	if(server)
	{
		udpUpSocket.bind(QHostAddress::Any,
		                 QSettings().value("network/port").toUInt());
		auto socketPtr(&udpUpSocket);
		auto clientsPtr(&clients);
		auto netTimerPtr(&networkTimer);
		connect(socketPtr, &QUdpSocket::readyRead,
		        [socketPtr, clientsPtr, netTimerPtr]() {
			        while(socketPtr->hasPendingDatagrams())
			        {
				        QNetworkDatagram datagram(socketPtr->receiveDatagram());

				        QString str;
				        quint16 p;
				        qreal ft;
				        quint16 cid;
				        QByteArray buf(datagram.data());
				        QDataStream stream(&buf, QIODevice::ReadOnly);
				        stream >> str;
				        stream >> p;
				        stream >> ft;
				        stream >> cid;
				        if(str != PROJECT_NAME)
				        {
					        qDebug() << "Received bad header :";
					        qDebug() << str;
					        continue;
				        }

				        bool exists(false);
				        for(auto& c : *clientsPtr)
				        {
					        if(c.clientId == cid)
					        {
						        c.frameTiming      = ft;
						        c.clientId         = cid;
						        c.lastReceivedTime = netTimerPtr->elapsed();
						        qDebug() << "Update client :";
						        qDebug() << c.addr;
						        qDebug() << c.port;
						        qDebug() << c.lastReceivedTime;
						        qDebug() << c.clientId;
						        exists = true;
						        break;
					        }
				        }
				        if(!exists)
				        {
					        auto tcpSock = new QTcpSocket;
					        clientsPtr->append({datagram.senderAddress(), p, ft,
					                            cid, netTimerPtr->elapsed(), 0,
					                            tcpSock});
					        tcpSock->connectToHost(
					            datagram.senderAddress(),
					            QSettings().value("network/tcpport").toUInt());
					        qDebug() << "New client :";
					        qDebug() << datagram.senderAddress();
					        qDebug() << p;
					        qDebug() << cid;
				        }
			        }
		        });
	}
	// UDP client
	else
	{
		udpDownSocket.bind(QSettings().value("network/ip").toUInt());
		auto socketPtr(&udpDownSocket);
		auto netStatePtr(networkedState);
		connect(socketPtr, &QAbstractSocket::readyRead,
		        [socketPtr, netStatePtr]() {
			        QNetworkDatagram datagram(socketPtr->receiveDatagram());

			        QByteArray buf(datagram.data());
			        QDataStream stream(&buf, QIODevice::ReadOnly);
			        netStatePtr->readFromDataStream(stream);
		        });

		tcpServer = new QTcpServer(this);
		tcpServer->listen(QHostAddress::Any,
		                  QSettings().value("network/tcpport").toUInt());
		connect(tcpServer, &QTcpServer::newConnection, [this]() {
			tcpSocket = tcpServer->nextPendingConnection();
			if(!tcpSocket->peerAddress().isEqual(
			       QHostAddress(QSettings().value("network/ip").toString())))
			{
				return;
			}
			connect(tcpSocket, &QTcpSocket::readyRead, [this]() {
				PythonQtHandler::evalScript(tcpSocket->readAll());
			});
		});
	}
	networkTimer.start();
}

void NetworkManager::sendPythonScript(unsigned int toClientId,
                                      QString const& script) const
{
	if(!isServer())
	{
		qWarning() << "Attempting to send python script to another client from "
		              "a client. Use the server instance.";
	}
	for(auto const& c : clients)
	{
		if(c.clientId == toClientId)
		{
			c.tcpSocket->write(script.toLatin1());
		}
	}
}

void NetworkManager::update(float frameTiming)
{
	if(networkedState == nullptr)
	{
		return;
	}
	if(server)
	{
		QByteArray buf;
		QDataStream stream(&buf, QIODevice::WriteOnly);
		networkedState->writeInDataStream(stream);

		// qDebug() << "Sending " + QString::number(buf.size()) + " bytes.";

		for(int i(clients.size() - 1); i >= 0; --i)
		{
			// if hasn't responded in 10 seconds, "disconnect"
			if(networkTimer.elapsed() - clients.at(i).lastReceivedTime > 10000)
			{
				qDebug() << "Client disconnected :";
				qDebug() << clients.at(i).addr;
				qDebug() << clients.at(i).port;
				delete clients[i].tcpSocket;
				clients.removeAt(i);
			}
			else if(networkTimer.elapsed() - clients.at(i).lastSentTime
			        > clients.at(i).frameTiming * 1500)
			{
				udpDownSocket.writeDatagram(buf, clients.at(i).addr,
				                            clients.at(i).port);
				clients[i].lastSentTime = networkTimer.elapsed();
			}
		}
	}
	else
	{
		if(networkTimer.elapsed() > 1000)
		{
			QByteArray buf;
			QDataStream stream(&buf, QIODevice::WriteOnly);
			stream << QString(PROJECT_NAME);
			stream << quint16(udpDownSocket.localPort());
			stream << qreal(frameTiming);
			stream << quint16(clientId);
			udpUpSocket.writeDatagram(
			    buf, QHostAddress(QSettings().value("network/ip").toString()),
			    QSettings().value("network/port").toUInt());
			networkTimer.restart();
		}
	}
}

NetworkManager::~NetworkManager()
{
	if(server)
	{
		for(auto const& c : clients)
		{
			delete c.tcpSocket;
		}
	}
	else
	{
		delete tcpServer;
	}
	delete networkedState;
}

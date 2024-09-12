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

NetworkManager::NetworkManager(std::unique_ptr<AbstractState> networkedState)
    : networkedState(std::move(networkedState))
{
	if(this->networkedState == nullptr)
	{
		return;
	}
	if(server)
	{
		udpUpSocket.bind(QHostAddress::Any,
		                 QSettings().value("network/port").toUInt());
		connect(&udpUpSocket, &QUdpSocket::readyRead,
		        [this]()
		        {
			        while(udpUpSocket.hasPendingDatagrams())
			        {
				        const QNetworkDatagram datagram(
				            udpUpSocket.receiveDatagram());

				        QString str;
				        quint16 p   = 0;
				        qreal ft    = NAN;
				        quint16 cid = 0;
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
				        for(auto& c : clients)
				        {
					        if(c.clientId == cid)
					        {
						        c.frameTiming      = ft;
						        c.clientId         = cid;
						        c.lastReceivedTime = networkTimer.elapsed();
						        /*qDebug() << "Update client :";
						        qDebug() << c.addr;
						        qDebug() << c.port;
						        qDebug() << c.lastReceivedTime;
						        qDebug() << c.clientId;*/
						        exists = true;
						        break;
					        }
				        }
				        if(!exists)
				        {
					        clients.emplace_back(datagram.senderAddress(), p,
					                             ft, cid,
					                             networkTimer.elapsed());
					        clients.back().tcpSocket.connectToHost(
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
		connect(&udpDownSocket, &QAbstractSocket::readyRead,
		        [this]()
		        {
			        const QNetworkDatagram datagram(
			            udpDownSocket.receiveDatagram());

			        QByteArray buf(datagram.data());
			        QDataStream stream(&buf, QIODevice::ReadOnly);
			        this->networkedState->readFromDataStream(stream);
		        });

		tcpServer = std::make_unique<QTcpServer>(this);
		tcpServer->listen(QHostAddress::Any,
		                  QSettings().value("network/tcpport").toUInt());
		connect(tcpServer.get(), &QTcpServer::newConnection,
		        [this]()
		        {
			        auto* pendingConn = tcpServer->nextPendingConnection();
			        if(!pendingConn->peerAddress().isEqual(QHostAddress(
			               QSettings().value("network/ip").toString())))
			        {
				        return;
			        }
			        connect(pendingConn, &QTcpSocket::readyRead,
			                [pendingConn]() {
				                PythonQtHandler::evalScript(
				                    pendingConn->readAll());
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
			c.tcpSocket.write(script.toLatin1());
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

		for(auto it = clients.begin(); it != clients.end();)
		{
			// if hasn't responded in 10 seconds, "disconnect"
			if(networkTimer.elapsed() - it->lastReceivedTime > 10000)
			{
				qDebug() << "Client disconnected :";
				qDebug() << it->addr;
				qDebug() << it->port;
				it = clients.erase(it);
			}
			else if(networkTimer.elapsed() - it->lastSentTime
			        > it->frameTiming * 1500)
			{
				udpDownSocket.writeDatagram(buf, it->addr, it->port);
				it->lastSentTime = networkTimer.elapsed();
				++it;
			}
			else
			{
				++it;
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
			stream << udpDownSocket.localPort();
			stream << static_cast<qreal>(frameTiming);
			stream << static_cast<quint16>(clientId);
			udpUpSocket.writeDatagram(
			    buf, QHostAddress(QSettings().value("network/ip").toString()),
			    QSettings().value("network/port").toUInt());
			networkTimer.restart();
		}
	}
}

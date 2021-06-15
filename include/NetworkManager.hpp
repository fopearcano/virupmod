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

#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include <QElapsedTimer>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

#include "AbstractState.hpp"
#include "PythonQtHandler.hpp"

class NetworkManager : public QObject
{
	Q_OBJECT
  public:
	// takes ownership
	explicit NetworkManager(AbstractState* networkedState);
	bool isServer() const { return server; };
	AbstractState* getNetworkedState() { return networkedState; };
	void sendPythonScript(unsigned int toClientId, QString const& script) const;
	void update(float frameTiming);
	~NetworkManager();

	const unsigned int clientId
	    = QSettings().value("network/clientid").toUInt();

  private:
	const bool server = QSettings().value("network/server").toBool();
	QUdpSocket udpUpSocket;   // client to server
	QUdpSocket udpDownSocket; // server to client
	QElapsedTimer networkTimer;

	// client
	AbstractState* networkedState = nullptr; // state to share
	QTcpServer* tcpServer         = nullptr;
	QTcpSocket* tcpSocket         = nullptr;
	// server
	struct Client
	{
		QHostAddress addr;
		quint16 port;
		qreal frameTiming;
		quint16 clientId;
		qint64 lastReceivedTime;
		qint64 lastSentTime;
		QTcpSocket* tcpSocket;
	};
	QList<Client> clients;
};

#endif // NETWORKMANAGER_HPP

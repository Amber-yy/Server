#include "Server.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <functional>

struct Server::Data
{
	QTcpServer *server;
	QTcpSocket *blueClient;
	QTcpSocket *redClient;
	QTextEdit *info;
	QComboBox *mode;
	QLabel *label;
	QPoint blueTeam;
	QPoint redTeam;
	QPoint blueHero;
	QPoint redHero;
	QPoint blueBuff1;
	QPoint redBuff1;
	QPoint blueBuff2;
	QPoint redBuff2;
	QPoint Nashor;
	QPoint Dragon;
	bool gaming;
};

static void updatePositon(QPoint &hero,int key);
static void resetRedKey(int &key,int index);

Server::Server(QWidget *parent): QWidget(parent)
{
	data = new Data;
	data->server = new QTcpServer(this);
	data->blueClient = nullptr;
	data->redClient = nullptr;

	data->label = new QLabel(u8"选择游戏模式：",this);
	data->label->setGeometry(0,2,120,26);

	data->mode = new QComboBox(this);
	data->mode->addItem(u8"英雄联盟/Dota模式");
	data->mode->addItem(u8"王者荣耀模式");
	data->mode->addItem(u8"决战平安京模式");
	data->mode->setCurrentIndex(0);
	data->mode->setGeometry(120, 2, 150, 26);

	data->info = new QTextEdit(this);
	data->info->setGeometry(0, 30, 270, 480);
	data->info->setEnabled(false);
	data->gaming = false;

	if (!data->server->listen(QHostAddress::Any, 9754))
	{
		data->server->close();
		QMessageBox::warning(this, u8"错误", "无法绑定端口");
		exit(0);
	}

	connect(data->server, &QTcpServer::newConnection, this,&Server::onNewConnection);
	connect(data->mode, &QComboBox::currentTextChanged, [this](const QString &)
	{
		if (data->gaming)
		{
			sendToBlue();
			sendToRed();
		}
	}
	);

	setMaximumSize(270, 480);
	setMinimumSize(270, 480);

	data->blueTeam = QPoint(40, 600);
	data->blueHero = QPoint(40, 500);
	data->blueBuff1 = QPoint(180, 320);
	data->redBuff1 = QPoint(640-320, 640-180);

	data->redTeam = QPoint(600, 40);
	data->redHero = QPoint(600, 140);
	data->blueBuff2 = QPoint(640-180, 640-320);
	data->redBuff2 = QPoint(320,180);

	data->Nashor = QPoint(160, 160);
	data->Dragon = QPoint(480, 480);

}

Server::~Server()
{
	delete data;
}

void Server::onNewConnection()
{
	QTcpSocket *client = data->server->nextPendingConnection();
	connect(client, &QTcpSocket::disconnected, client, &QObject::deleteLater);

	if (data->blueClient)
	{
		if (data->redClient)
		{
			client->disconnected();
			return;
		}
		data->redClient = client;
		connect(client, &QTcpSocket::disconnected, [this]() {data->redClient = nullptr; data->gaming = false; });
		connect(client, &QTcpSocket::readyRead, this, &Server::readFromRed);
		data->info->append(u8"一个客户端已经连接，游戏开始");
		data->gaming = true;
	}
	else
	{
		data->blueClient = client;
		connect(client, &QTcpSocket::disconnected, [this]() {data->blueClient = nullptr; data->gaming = false; });
		connect(client, &QTcpSocket::readyRead, this, &Server::readFromBlue);
		data->info->append(u8"一个客户端已经连接");
	}
}

void Server::readFromBlue()
{
	QByteArray result = data->blueClient->readAll();

	if (!data->gaming)
	{
		return;
	}

	int key = result[0];
	updatePositon(data->blueHero, key);

	sendToBlue();
	sendToRed();
}

void Server::readFromRed()
{
	QByteArray result = data->redClient->readAll();

	if (!data->gaming)
	{
		return;
	}

	int key = result[0];
	resetRedKey(key, data->mode->currentIndex());
	updatePositon(data->redHero, key);

	sendToBlue();
	sendToRed();
}

void Server::sendToBlue()
{
	QString pos = QString::number(data->blueTeam.x())+"+" +QString::number(data->blueTeam.y())
		+ "+" + QString::number(data->blueHero.x()) + "+" + QString::number(data->blueHero.y())
		+ "+" + QString::number(data->blueBuff1.x()) + "+" + QString::number(data->blueBuff1.y())
		+ "+" + QString::number(data->redBuff1.x()) + "+" + QString::number(data->redBuff1.y())
		+ "+" + QString::number(data->redTeam.x()) + "+" + QString::number(data->redTeam.y())
		+ "+" + QString::number(data->redHero.x()) + "+" + QString::number(data->redHero.y())
		+ "+" + QString::number(data->blueBuff2.x()) + "+" + QString::number(data->blueBuff2.y())
		+ "+" + QString::number(data->redBuff2.x()) + "+" + QString::number(data->redBuff2.y())
		+ "+" + QString::number(data->Nashor.x()) + "+" + QString::number(data->Nashor.y())
		+ "+" + QString::number(data->Dragon.x()) + "+" + QString::number(data->Dragon.y());

	data->blueClient->write(pos.toLatin1());
	data->blueClient->flush();

	QStringList list = pos.split('+');

	QString msg = u8"蓝色方基地位置：(" +list[0]+","+list[1]+")\n";
	msg += u8"蓝色方英雄位置：(" + list[2] + "," + list[3] + ")\n";
	msg += u8"蓝色方蓝buff位置：(" + list[4] + "," + list[5] + ")\n";
	msg += u8"蓝色方红buff位置：(" + list[6] + "," + list[7] + ")\n";
	msg += u8"红色方基地位置：(" + list[8] + "," + list[9] + ")\n";
	msg += u8"红色方英雄位置：(" + list[10] + "," + list[11] + ")\n";
	msg += u8"红色方蓝buff位置：(" + list[12] + "," + list[13] + ")\n";
	msg += u8"红色方红buff位置：(" + list[14] + "," + list[15] + ")\n";
	msg += u8"大龙位置：(" + list[16] + "," + list[17] + ")\n";
	msg += u8"小龙位置：(" + list[18] + "," + list[19] + ")\n";

	data->info->setText(msg);
}

void Server::sendToRed()
{
	auto mode0 = [](QPoint &p)->QPoint 
	{
		return p; 
	};

	auto mode1 = [](QPoint &p)->QPoint 
	{
		QPoint t(640 - p.x(), 640 - p.y());
		return t; 
	};

	auto mode2 = [](QPoint &p)->QPoint
	{
		QPoint t(p.y(),p.x());
		return t;
	};

	std::function<QPoint(QPoint &)> tran;
	int index = data->mode->currentIndex();
	
	if (index == 0)
	{
		tran = mode0;
	}
	else if (index == 1)
	{
		tran = mode1;
	}
	else
	{
		tran = mode2;
	}

	QString pos = QString::number(tran(data->redTeam).x()) + "+" + QString::number(tran(data->redTeam).y())
		+ "+" + QString::number(tran(data->redHero).x()) + "+" + QString::number(tran(data->redHero).y())
		+ "+" + QString::number(tran(data->blueBuff1).x()) + "+" + QString::number(tran(data->blueBuff1).y())
		+ "+" + QString::number(tran(data->redBuff1).x()) + "+" + QString::number(tran(data->redBuff1).y())
		+ "+" + QString::number(tran(data->blueTeam).x()) + "+" + QString::number(tran(data->blueTeam).y())
		+ "+" + QString::number(tran(data->blueHero).x()) + "+" + QString::number(tran(data->blueHero).y())
		+ "+" + QString::number(tran(data->blueBuff2).x()) + "+" + QString::number(tran(data->blueBuff2).y())
		+ "+" + QString::number(tran(data->redBuff2).x()) + "+" + QString::number(tran(data->redBuff2).y())
		+ "+" + QString::number(tran(data->Nashor).x()) + "+" + QString::number(tran(data->Nashor).y())
		+ "+" + QString::number(tran(data->Dragon).x()) + "+" + QString::number(tran(data->Dragon).y());

	data->redClient->write(pos.toLatin1());
	data->redClient->flush();
}

void updatePositon(QPoint &hero,int key)
{
	if (key == 'W')
	{
		if (hero.y() > 0)
		{
			hero.setY(hero.y() - 1);
		}
	}
	else if (key == 'A')
	{
		if (hero.x() > 0)
		{
			hero.setX(hero.x() - 1);
		}
	}
	else if (key == 'S')
	{
		if (hero.y() < 640)
		{
			hero.setY(hero.y() + 1);
		}
	}
	else if (key == 'D')
	{
		if (hero.x() < 640)
		{
			hero.setX(hero.x() + 1);
		}
	}
}

static void resetRedKey(int &key, int index)
{
	if (index == 1)
	{
		if (key == 'W')
		{
			key = 'S';
		}
		else if (key == 'S')
		{
			key = 'W';
		}
		else if (key == 'A')
		{
			key = 'D';
		}
		else
		{
			key = 'A';
		}
	}
	else if(index==2)
	{
		if (key == 'W')
		{
			key = 'A';
		}
		else if (key == 'S')
		{
			key = 'D';
		}
		else if(key == 'A')
		{
			key = 'W';
		}
		else
		{
			key = 'S';
		}
	}
}
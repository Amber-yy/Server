#pragma once

#include <QWidget>

class Server : public QWidget
{
	Q_OBJECT
public:
	Server(QWidget *parent = Q_NULLPTR);
	~Server();
protected:
	void onNewConnection();
	void readFromBlue();
	void readFromRed();
	void sendToBlue();
	void sendToRed();
protected:
	struct Data;
	Data *data;
};

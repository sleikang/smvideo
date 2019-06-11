#include "DownloadThread.h"
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>
#include <QtNetwork>
#include <QFile>

DownloadThread::DownloadThread(QObject *parent)
	: QObject(parent)
{
	request = new QNetworkRequest;
	manager = new QNetworkAccessManager(this);
	connect(manager, &QNetworkAccessManager::finished, this, &DownloadThread::readUrlValue);
}

DownloadThread::~DownloadThread()
{
	manager->deleteLater();
	delete request;
	qDeleteAll(fileList);
	fileList.clear();
}

//设置字幕文件名
void DownloadThread::setFileName(const QString & str)
{
	fileName = str;
}

//搜索字幕
void DownloadThread::run()
{
	qDeleteAll(fileList);
	fileList.clear();
	QString name = fileName.split("/").at(fileName.split("/").size() - 1);
	name.replace(QRegularExpression("(\\d\\d\\d\\d.*?$|WEB.*?$|\\d{3,4}(P|p).*?$|HD.*?$|BD.*?$|HC.*?$)"), "").replace(QRegularExpression("(\\.)\\S{3,4}$"), "").replace(QRegularExpression(QStringLiteral("(\\.)|-|_|[(]|[)]|（|）")), " ").replace(QRegularExpression(QStringLiteral("\\s+$")), "");
	QString url = QString("http://api.assrt.net/v1/sub/search?token=xCgJoneIYWoS13ZpYQSkTGUi8O1dv3ii&q=%1&cnt=15&pos=0").arg(name);
	request->setUrl(QUrl(url));
	manager->get(*request);
	readNum = 0;
}

//设置下载模式
void DownloadThread::setDownMode(int mode)
{
	downMode = mode;
}

//下载选中字幕
void DownloadThread::loadRowDown(int row)
{
	readNum = 1;
	QString url = QString("http://api.assrt.net/v1/sub/detail?token=xCgJoneIYWoS13ZpYQSkTGUi8O1dv3ii&id=%1").arg(fileList[row]->at(0));
	request->setUrl(QUrl(url));
	manager->get(*request);
}


//返回数据
void DownloadThread::readUrlValue(QNetworkReply *reply)
{
	if (readNum == 0) {
		QString strAll = reply->readAll();
		QByteArray allData = strAll.toUtf8();
		QJsonParseError jsonError;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(allData, &jsonError);
		if (jsonError.error == QJsonParseError::NoError) {
			QJsonObject rootObject = jsonDoc.object();
			if (rootObject.contains("status") && rootObject.value("status").toInt() == 0) {
				if (rootObject.contains("sub")) {
					QJsonObject root_1 = rootObject.value("sub").toObject();
					if (root_1.contains("subs")) {
						QJsonArray jsonArray = root_1.value("subs").toArray();
						for (int i = 0; i < jsonArray.count(); i++) {
							QJsonObject childObject = jsonArray[i].toObject();
							fileList.append(new QStringList);
							if (childObject.contains("id")) {
								QJsonValue jsonValue = childObject.value("id");
								*fileList[i] << QString::number(jsonValue.toInt());
							}

							QString name;
							if (childObject.contains("lang")) {
								QJsonObject root_2 = childObject.value("lang").toObject();
								if (root_2.contains("desc")) {
									QJsonValue jsonValue = root_2.value("desc");
									name = jsonValue.toString();
									name.insert(0, "[").append("]");
								}

							}

							if (childObject.contains("native_name") && childObject.value("native_name").toString() != "") {
								QJsonValue jsonValue = childObject.value("native_name");
								name += jsonValue.toString();
							}
							else if (childObject.contains("videoname")) {
								QJsonValue jsonValue = childObject.value("videoname");
								name += jsonValue.toString();
							}

							*fileList[i] << name;
						}


						if (fileList.size() > 0) {
							if (downMode == 0) {
								readNum = 1;
								QString url = QString("http://api.assrt.net/v1/sub/detail?token=xCgJoneIYWoS13ZpYQSkTGUi8O1dv3ii&id=%1").arg(fileList[0]->at(0));
								request->setUrl(QUrl(url));
								manager->get(*request);
								reply->deleteLater();
								return;
							}
							else if (downMode == 1) {
								emit readListWidget(fileList);
							}
						}
						else {
							emit loadStatus(QStringLiteral("字幕未找到"));
						}
					}
				}
			}
			else {
				emit loadStatus(QStringLiteral("字幕查询失败"));
			}
		}
	}
	else if (readNum == 1) {
		QString strAll = reply->readAll();
		QByteArray allData = strAll.toUtf8();
		QJsonParseError jsonError;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(allData, &jsonError);
		if (jsonError.error == QJsonParseError::NoError) {
			QJsonObject rootObject = jsonDoc.object();
			if (rootObject.contains("status") && rootObject.value("status").toInt() == 0) {
				if (rootObject.contains("sub")) {
					QJsonObject root_1 = rootObject.value("sub").toObject();
					if (root_1.contains("subs")) {
						QJsonArray jsonArray = root_1.value("subs").toArray();
						for (int i = 0; i < jsonArray.count(); i++) {
							QJsonObject root_2 = jsonArray[i].toObject();
							if (root_2.contains("filelist")) {
								QJsonArray tempArray = root_2.value("filelist").toArray();
								for (int j = 0; j < tempArray.count(); j++) {
									QJsonObject childObject = tempArray[j].toObject();
									if (childObject.contains("f")) {
										QJsonValue jsonValue = childObject.value("f");
										QRegularExpression rx("(\\.)\\S{1,3}$");
										QRegularExpressionMatch match = rx.match(jsonValue.toString());
										if (match.hasMatch()) {
											fileType = match.captured(0);
										}
									}

									if (childObject.contains("url")) {
										QJsonValue jsonValue = childObject.value("url");
										readNum = 2;
										request->setUrl(QUrl(jsonValue.toString()));
										manager->get(*request);
										reply->deleteLater();
										return;
									}
								}
							}
						}
					}
				}
			}
			else {
				emit loadStatus(QStringLiteral("字幕查询失败"));
			}
		}
	}
	else if (readNum == 2) {
		QString inName = fileName;
		inName.replace(QRegularExpression("(\\.)\\S{1,4}$"), fileType);
		QFile file(inName);
		if (!file.open(QIODevice::WriteOnly)) {
			emit loadStatus(QStringLiteral("无法创建文件"));
			file.close();
			goto bk;
		}
		file.write(reply->readAll());
		file.close();
		emit loadStatus(QStringLiteral("字幕下载完成"));
		emit readFileName(inName);
	}

bk:	reply->deleteLater();
}
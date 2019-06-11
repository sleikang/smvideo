#include "DownMovieInfo.h"
#include <qnetworkaccessmanager.h>
#include <qregularexpression.h>
#include <QNetworkReply>
#include <QtNetwork>
#include <qlabel.h>

DownMovieInfo::DownMovieInfo(QObject *parent)
	: QObject(parent)
{

}

DownMovieInfo::DownMovieInfo(QLabel *la, QLabel *lb, QObject *parent)
	: labelImage(la), labelInfo(lb), QObject(parent)
{
	request = new QNetworkRequest;
	manager = new QNetworkAccessManager(this);
	connect(manager, &QNetworkAccessManager::finished, this, &DownMovieInfo::movieInfo);
}

DownMovieInfo::~DownMovieInfo()
{
	manager->deleteLater();
	delete request;
}

//开始查询电影信息
void DownMovieInfo::run()
{
	fileName.replace(QRegularExpression("(WEB.*?$|\\d{3,4}(P|p).*?$|HD.*?$|BD.*?$|HC.*?$)"), "").replace(QRegularExpression("(\\.)\\S{3,4}$"), "").replace(QRegularExpression(QStringLiteral("(\\.)|-|_|[(]|[)]|（|）")), " ").replace(QRegularExpression(QStringLiteral("\\s+$")), "");
	QString url = QString("https://frodo.douban.com/api/v2/search?q=%1&apiKey=054022eaeae0b00e0fc068c0c0a2102a&count=1").arg(fileName);
	request->setUrl(QUrl(url));
	request->setRawHeader("Referer", "https://servicewechat.com/wx2f9b06c1de1ccfca/41/page-frame.html");
	manager->get(*request);
	readNum = 0;
}

//设置文件名
void DownMovieInfo::setFileName(const QString & str)
{
	fileName = str;
}

//返回查询信息解析
void DownMovieInfo::movieInfo(QNetworkReply *reply)
{
	if (readNum == 0) {
		QByteArray allData = reply->readAll();
		QJsonParseError jsonError;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(allData, &jsonError);
		if (jsonError.error == QJsonParseError::NoError) {
			QJsonObject rootObject = jsonDoc.object();
			if (rootObject.contains("subjects")) {
				QJsonArray jsonArray = rootObject.value("subjects").toArray();
				for (int i = 0; i < jsonArray.count(); i++) {
					QJsonObject childObject = jsonArray[i].toObject();
					if (childObject.contains("title")) {
						QJsonValue jsonValue = childObject.value("title");
						fileName = jsonValue.toString();
						info = QStringLiteral("标题: %1\n").arg(jsonValue.toString());
					}

					if (childObject.contains("year")) {
						QJsonValue jsonValue = childObject.value("year");
						fileName += " " + jsonValue.toString();
						info += QStringLiteral("年代: %1\n").arg(jsonValue.toString());
					}

					if (childObject.contains("id")) {
						QJsonValue jsonValue = childObject.value("id");
						readNum = 1;
						QString url = QString("https://frodo.douban.com/api/v2/movie/%1?apiKey=054022eaeae0b00e0fc068c0c0a2102a").arg(jsonValue.toString());
						request->setUrl(QUrl(url));
						manager->get(*request);
						reply->deleteLater();
						return;
					}
				}
			}
		}
		emit loadStatus(QStringLiteral("未找到电影相关信息"));
	}
	else if (readNum == 1) {
		QByteArray allData = reply->readAll();
		QJsonParseError jsonError;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(allData, &jsonError);
		if (jsonError.error == QJsonParseError::NoError) {
			QJsonObject rootObject = jsonDoc.object();
			if (rootObject.contains("durations")) {
				QJsonArray jsonArray = rootObject.value("durations").toArray();
				QString temp;
				for (int i = 0; i < jsonArray.count(); i++) {
					if (!temp.isEmpty())
						temp += " ";
					temp += jsonArray[i].toString();
				}
				info += QStringLiteral("时长: %1\n").arg(temp);
			}

			if (rootObject.contains("languages")) {
				QJsonArray jsonArray = rootObject.value("languages").toArray();
				QString temp;
				for (int i = 0; i < jsonArray.count(); i++) {
					if (!temp.isEmpty())
						temp += " ";
					temp += jsonArray[i].toString();
				}
				info += QStringLiteral("语言: %1\n").arg(temp);
			}

			if (rootObject.contains("genres")) {
				QJsonArray jsonArray = rootObject.value("genres").toArray();
				QString temp;
				for (int i = 0; i < jsonArray.count(); i++) {
					if (!temp.isEmpty())
						temp += " ";
					temp += jsonArray[i].toString();
				}
				info += QStringLiteral("类型: %1\n").arg(temp);
			}

			if (rootObject.contains("rating")) {
				QJsonObject childObject = rootObject.value("rating").toObject();
				if (childObject.contains("value")) {
					QJsonValue jsonValue = childObject.value("value");
					info += QStringLiteral("评分: %1\n").arg(jsonValue.toDouble());
				}
			}

			if (rootObject.contains("intro")) {
				QJsonValue jsonValue = rootObject.value("intro");
				plot = jsonValue.toString();
			}


			if (rootObject.contains("pic")) {
				QJsonObject childObject = rootObject.value("pic").toObject();
				if (childObject.contains("normal")) {
					QJsonValue jsonValue = childObject.value("normal");
					readNum = 2;
					request->setUrl(QUrl(jsonValue.toString()));
					manager->get(*request);
					reply->deleteLater();
					return;
				}
			}
		}

		emit loadStatus(QStringLiteral("未找到电影相关信息"));
	}
	else if (readNum == 2) {
		QPixmap pixmap;
		pixmap.loadFromData(reply->readAll());
		emit readTextPlot(plot);
		labelImage->setPixmap(pixmap);
		labelInfo->setText(info);
		emit loadStatus(QStringLiteral("电影信息查询完成"));
	}

	reply->deleteLater();
	emit loadFinish();
}
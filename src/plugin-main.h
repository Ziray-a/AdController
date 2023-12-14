#ifndef PLUGIN_MAIN_H_
#define PLUGIN_MAIN_H_
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <string>
#include <QComboBox>
#include <QLineEdit>

typedef std::tuple<int, std::string> AdInfo;

class AdControlWidget : public QWidget {
	Q_OBJECT

public:
	AdControlWidget();
	void setURL(std::string url);
	std::string getURL();

private:
	void updateAds();
	std::string URL;
	void getAds(std::string URL);
	std::vector<AdInfo> availableAds;
	void reloadAds();
	void playAd();
	int chosenAd;
	void setAd();
	QPushButton *refresh = new QPushButton(tr("&Refresh"));
	QLineEdit *APIlink = new QLineEdit();
	QVBoxLayout *vbox = new QVBoxLayout();
	QComboBox *adSelection = new QComboBox();
	QPushButton *adPlayButton = new QPushButton(tr("&Play Ad"));
};

#endif // PLUGIN_MAIN_H_
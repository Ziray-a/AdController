#ifndef PLUGIN_MAIN_H_
#define PLUGIN_MAIN_H_
#include <obs-module.h>
#include <plugin-support.h>
#include <plugin-main.h>
#include <obs-frontend-api.h>
#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include <string>
#include <QComboBox>
#include <QLineEdit>
#include <QDialog>
#include <QLabel>
typedef std::tuple<int, std::string> AdInfo;

class SettingsWindow : public QDialog {
	Q_OBJECT
public:
	SettingsWindow(config_t *config);
	QLabel *apiHostLabel = new QLabel(tr(" API-Host: "));
	QLineEdit *hostLine = new QLineEdit();
	config_t *settingConfig;
	QGridLayout *layout = new QGridLayout(this);
	QGridLayout *childLayout = new QGridLayout();
	QPushButton *cancel = new QPushButton(tr("&Cancel"));
	QPushButton *ok = new QPushButton(tr("&OK"));
	void cancelclose();
	void okayclose();
};
class SettingsButton : public QPushButton {
	Q_OBJECT

public:
	SettingsButton();
	void ButtonClicked();
};

class AdControlWidget : public QWidget {
	Q_OBJECT

public:
	AdControlWidget(std::string url);
	void setURL(std::string url);
	std::string getURL();
	void reloadAds();

private:
	void updateAds();
	std::string URL;
	void getAds(std::string URL);
	std::vector<AdInfo> availableAds;
	void playAd();
	int chosenAd;
	void setAd();
	QPushButton *refresh = new QPushButton(tr("&Refresh"));
	QLineEdit *APIlink = new QLineEdit();
	QGridLayout *upperGrid = new QGridLayout();
	QGridLayout *middleGrid = new QGridLayout();
	QGridLayout *bottomGrid = new QGridLayout();
	QGridLayout *parentGrid = new QGridLayout();
	QComboBox *adSelection = new QComboBox();
	QPushButton *adPlayButton = new QPushButton(tr("&Play Ad"));
	QPushButton *settingsButton = new SettingsButton();
};

#endif // PLUGIN_MAIN_H_
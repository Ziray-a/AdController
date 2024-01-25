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

typedef std::tuple<int, std::string, std::string> AdInfo;

class SettingsWindow : public QDialog {
	Q_OBJECT
public:
	SettingsWindow(config_t *config);
	QLabel *apiHostLabel = new QLabel(tr(" API-Host: "));
	QLineEdit *hostLine = new QLineEdit();
	QLabel *apiTokenLabel = new QLabel(tr(" API-Token: "));
	QLineEdit *tokenLine = new QLineEdit();
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
	AdControlWidget(std::string url, std::string ptoken);
	void setURL(std::string url);
	void setToken(const std::string &ptoken);
	std::string getToken();
	std::string getURL();
	std::string videoLink;
	void reloadAds();
	void loadVideo();
	obs_source *prevscene;
	obs_scene *adScene;
	obs_source *scenesource;
	obs_data *mediasettings;
	obs_source *adsource;
	QPushButton *adPlayButton = new QPushButton(tr("&Play Ad"));

private:
	void getAdLink(std::string APIHost, int adID);
	void updateAds();
	std::string URL;
	std::string token;
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
	QPushButton *settingsButton = new SettingsButton();
};

#endif // PLUGIN_MAIN_H_

#ifndef PLUGIN_MAIN_H_
#define PLUGIN_MAIN_H_
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <string>
#include <QComboBox>
#include <QLineEdit>
#include <QDialog>

typedef std::tuple<int, std::string> AdInfo;

class SettingsWindow : public QDialog {
	Q_OBJECT
public:
	SettingsWindow();
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
	QGridLayout *upperGrid = new QGridLayout();
	QGridLayout *middleGrid = new QGridLayout();
	QGridLayout *bottomGrid = new QGridLayout();
	QGridLayout *parentGrid = new QGridLayout();
	QComboBox *adSelection = new QComboBox();
	QPushButton *adPlayButton = new QPushButton(tr("&Play Ad"));
	QPushButton *settingsButton = new SettingsButton();
};

#endif // PLUGIN_MAIN_H_
/*
Plugin Name
Copyright (C) 2023 sodgeIT kontakt@sodgeit.de

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <plugin-support.h>
#include <plugin-main.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>
#include <callback/signal.h>
#include <QDockWidget>
#include <QMenuBar>
#include <QComboBox>
#include <thread>
#include <chrono>
#include <random>
#include <QLabel>
#include <iostream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <nlohmann/json.hpp>
#include <QFile>
#include <thread>
#include <chrono>

using json = nlohmann::json;
config_t *pluginConfig;
AdControlWidget *dockWidget;
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

std::vector<obs_source *> mutedItems;
bool obs_module_load(void)
{
	int conftest = config_open(&pluginConfig, "obs-ad-slice-controler.ini",
				   CONFIG_OPEN_EXISTING);
	if (conftest == CONFIG_FILENOTFOUND) {

		pluginConfig = config_create("obs-ad-slice-controler.ini");

		config_set_default_string(pluginConfig, "API", "API-Host",
					  "http://localhost:5499");
		config_set_default_string(pluginConfig, "API", "Token",
					  "Token Here");
		config_save_safe(pluginConfig, ".ex.tmp", ".ex.back");
		obs_log(LOG_INFO, "Created new Adslice Config");

	} else if (conftest == CONFIG_ERROR) {
		obs_log(LOG_INFO, "Error in config file - reseting...");
		pluginConfig = config_create("obs-ad-slice-controler.ini");
	}

	obs_log(LOG_INFO, "config found reading config");

	char pluginID[] = "obs-ad_slice_controller_100";
	dockWidget = new AdControlWidget(
		config_get_string(pluginConfig, "API", "API-Host"),
		config_get_string(pluginConfig, "API", "Token"));
	dockWidget->setMinimumHeight(200);
	dockWidget->setMinimumWidth(150);

	if (!obs_frontend_add_dock_by_id(pluginID, "Ad Control", dockWidget))
		obs_log(LOG_ERROR, "Could not load dock");
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);

	return true;
}

void AdControlWidget::setURL(std::string url)
{
	URL = url;
}
void obs_module_unload(void)
{
	config_close(pluginConfig);
	obs_log(LOG_INFO, "plugin unloaded");
}

void SettingsButton::ButtonClicked()
{
	SettingsWindow *settings = new SettingsWindow(pluginConfig);
	settings->show();
}

SettingsWindow::SettingsWindow(config_t *config)
{
	this->setMaximumHeight(150);
	this->setWindowTitle("Ad Control Settings");
	hostLine->setText(config_get_string(config, "API", "API-Host"));
	tokenLine->setText(config_get_string(config, "API", "Token"));
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setRowStretch(0, 0);
	layout->setColumnStretch(0, 0);
	layout->addWidget(apiHostLabel, 0, 0);
	layout->addWidget(hostLine, 0, 1);
	layout->addWidget(apiTokenLabel, 1, 0);
	layout->addWidget(tokenLine, 1, 1);
	childLayout->addWidget(cancel, 0, 0);
	childLayout->addWidget(ok, 0, 1);
	layout->addLayout(childLayout, 2, 1);
	connect(cancel, &QPushButton::released, this,
		&SettingsWindow::cancelclose);
	connect(ok, &QPushButton::released, this, &SettingsWindow::okayclose);
	this->setLayout(layout);
}
void SettingsWindow::cancelclose()
{
	this->close();
}
void SettingsWindow::okayclose()
{
	config_set_string(pluginConfig, "API", "API-Host",
			  hostLine->text().toStdString().c_str());
	config_set_string(pluginConfig, "API", "Token",
			  tokenLine->text().toStdString().c_str());
	config_save_safe(pluginConfig, ".ex.tmp", ".ex.back");
	dockWidget->setURL(hostLine->text().toStdString());
	dockWidget->setToken(tokenLine->text().toStdString());
	dockWidget->reloadAds();
	this->close();
}

SettingsButton::SettingsButton()
{
	this->setText("&Settings");
	connect(this, &QPushButton::released, this,
		&SettingsButton::ButtonClicked);
}

AdControlWidget::AdControlWidget(std::string url, std::string ptoken)
{

	setURL(url);
	setToken(ptoken);
	upperGrid->setContentsMargins(0, 0, 0, 0);
	upperGrid->setRowStretch(0, 0);
	upperGrid->setColumnStretch(0, 0);
	middleGrid->setContentsMargins(0, 0, 0, 0);
	middleGrid->setRowStretch(0, 0);
	middleGrid->setColumnStretch(0, 0);
	this->upperGrid->addWidget(refresh, 0, 1);
	this->upperGrid->addWidget(adSelection, 0, 0);
	connect(adPlayButton, &QPushButton::released, this,
		&AdControlWidget::playAd);
	connect(refresh, &QPushButton::released, this,
		&AdControlWidget::reloadAds);
	this->middleGrid->addWidget(adPlayButton, 1, 0);
	bottomGrid->addWidget(settingsButton, 0, 7);
	parentGrid->addLayout(upperGrid, 0, 0);
	parentGrid->addLayout(middleGrid, 1, 0);
	parentGrid->addLayout(bottomGrid, 2, 0);
	this->setLayout(parentGrid);
	reloadAds();
}

bool muteSource(void *param, obs_source_t *source)
{
	auto whitelistsource = reinterpret_cast<obs_source_t *>(param);

	UNUSED_PARAMETER(param);
	std::cout << obs_source_get_name(source) << std::endl;
	if (obs_source_is_scene(source) || source == whitelistsource) {
		std::cout << obs_source_get_name(source);
		return true;
	} else {
		mutedItems.push_back(source);
		obs_source_set_muted(source, true);
		return true;
	}
}

void AdControlWidget::playAd()
{

	adPlayButton->setEnabled(false);
	QNetworkAccessManager manager;
	std::string url;
	getAdLink(getURL(),
		  std::get<0>(availableAds[adSelection->currentIndex()]));
}

static void finishAd(void *data, calldata_t *calldata)
{
	UNUSED_PARAMETER(calldata);
	AdControlWidget *widget = reinterpret_cast<AdControlWidget *>(data);

	for (auto it : mutedItems) {
		obs_source_set_muted(it, false);
	}

	obs_frontend_set_current_scene(widget->prevscene);
	obs_source_release(widget->prevscene);
	obs_source_release(widget->adsource);
	obs_source_remove(widget->adsource);
	obs_scene_prune_sources(widget->adScene);
	obs_scene_release(widget->adScene);
	obs_data_release(widget->mediasettings);
	obs_source_remove(widget->scenesource);
	widget->adPlayButton->setEnabled(true);
}

void AdControlWidget::loadVideo()
{
	using namespace std::chrono_literals;
	std::string videoJSONstring =
		"{\"input\": \"" + videoLink +
		"\",\"input_format\": \"mp4\", \"is_local_file\" : false, \"scale\": { \"x\": 1.5, \"y\": 1.5} }";
	prevscene = obs_frontend_get_current_scene();
	adScene = obs_scene_create("Ad Scene");
	scenesource = obs_scene_get_source(adScene);
	mediasettings = obs_data_create_from_json(videoJSONstring.c_str());
	adsource = obs_source_create("ffmpeg_source", "adSource", mediasettings,
				     NULL);
	obs_scene_add(adScene, adsource);
	obs_frontend_set_current_scene(scenesource);
	obs_enum_sources(muteSource, adsource);
	//play ad
	signal_handler_t *sh = obs_source_get_signal_handler(adsource);
	signal_handler_connect(sh, "media_ended", finishAd, this);
	//wait for ad to finish
}

void AdControlWidget::getAdLink(std::string APIHost, int adID)
{
	obs_log(LOG_INFO, "getting ad link ...");
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	std::string requestString;
	requestString.append(APIHost.c_str())
		.append("/prepareAd?adID=")
		.append(std::to_string(adID));
	std::cout << requestString << std::endl;
	QNetworkRequest *request = new QNetworkRequest(
		QUrl().fromPercentEncoding(requestString.c_str()));
	request->setRawHeader("User-Agent", "OBS QT 1.0");
	request->setRawHeader("Authorization", getToken().c_str());
	obs_log(LOG_INFO, "getting ad link ...");
	connect(manager, &QNetworkAccessManager::finished, this,
		[=](QNetworkReply *reply) {
			obs_log(LOG_INFO, "requesting ...");
			if (reply->error() == QNetworkReply::NoError) {
				QByteArray response = reply->readAll();
				std::string stringseps = response.toStdString();
				std::string returnstring;

				returnstring.append(APIHost.c_str())
					.append("/loadAd?adID=")
					.append(std::to_string(adID))
					.append("&dlID=")
					.append(stringseps.c_str());
				videoLink = returnstring;

			} else {
				std::cout << reply->readAll().toStdString()
					  << std::endl;
			}
			loadVideo();
		});
	manager->get(*request);
}

void AdControlWidget::getAds(std::string APIHost)
{
	obs_log(LOG_INFO, "getting Ads...");
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	std::string requestString;
	requestString.append(APIHost.c_str()).append("/getAds");
	QNetworkRequest *request = new QNetworkRequest(
		QUrl().fromPercentEncoding(requestString.c_str()));
	request->setRawHeader("User-Agent", "OBS QT 1.0");
	request->setRawHeader("Authorization", getToken().c_str());
	connect(manager, &QNetworkAccessManager::finished, this,
		[=](QNetworkReply *reply) {
			obs_log(LOG_INFO, "requesting ...");
			if (reply->error() == QNetworkReply::NoError) {
				QByteArray response = reply->readAll();
				std::string stringseps = response.toStdString();
				availableAds.clear();
				json respjson = json::parse(stringseps);
				for (auto &array : respjson) {
					availableAds.emplace_back(AdInfo(

						array["id"].get<int>(),
						array["name"]
							.get<std::string>()));
				}
				adPlayButton->setEnabled(true);

			} else {
				availableAds.clear();
				availableAds.emplace_back(AdInfo(
					0, reply->errorString().toStdString()));
				obs_log(LOG_ERROR, reply->errorString()
							   .toStdString()
							   .c_str());
				adPlayButton->setEnabled(false);
			}
			updateAds();
		});

	manager->get(*request);
}

std::string AdControlWidget::getURL()
{
	return URL;
}

std::string AdControlWidget::getToken()
{
	return this->token;
}

void AdControlWidget::setToken(const std::string &ptoken)
{
	this->token = ptoken;
}

void AdControlWidget::reloadAds()
{
	getAds(URL);
	updateAds();
}

void AdControlWidget::updateAds()
{
	adSelection->clear();
	for (AdInfo ad : availableAds) {
		adSelection->addItem(std::get<1>(ad).c_str(),
				     QVariant(std::get<0>(ad)));
	}
}

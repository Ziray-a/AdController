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

bool obs_module_load(void)
{
	if (config_open(&pluginConfig, "./obs-ad-slice-controler.ini",
			CONFIG_OPEN_EXISTING) == CONFIG_FILENOTFOUND) {

		pluginConfig = config_create("./obs-ad-slice-controler.ini");

		config_set_default_string(pluginConfig, "API", "API-Host",
					  "http://localhost:5499");
		config_save_safe(pluginConfig, ".ex.tmp", ".ex.back");
		obs_log(LOG_INFO, "Created new Adslice Config");
	} else if (config_open(&pluginConfig, "./obs-ad-slice-controler.ini",
			       CONFIG_OPEN_EXISTING) == CONFIG_ERROR) {
		obs_log(LOG_INFO, "Error in config file - reseting...");
		pluginConfig = config_create("./obs-ad-slice-controler.ini");
	}
	obs_log(LOG_INFO, "config found reading config");
	config_open(&pluginConfig, "./obs-ad-slice-controler.ini",
		    CONFIG_OPEN_EXISTING);
	const char *confurl =
		config_get_string(pluginConfig, "API", "API-Host");
	if (!confurl)
		confurl = "";
	obs_log(LOG_INFO, confurl);

	char pluginID[] = "obs-ad_slice_controller_100";
	dockWidget = new AdControlWidget(
		config_get_string(pluginConfig, "API", "API-Host"));
	dockWidget->setMinimumHeight(200);
	dockWidget->setMinimumWidth(150);

	if (!obs_frontend_add_dock_by_id(pluginID, "Ad Control", dockWidget))
		throw "Could not add dock for plugin";
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
	hostLine->setText(config_get_string(config, "API", "API-Host"));
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setRowStretch(0, 0);
	layout->setColumnStretch(0, 0);
	layout->addWidget(apiHostLabel, 0, 0);
	layout->addWidget(hostLine, 0, 1);
	childLayout->addWidget(cancel, 0, 0);
	childLayout->addWidget(ok, 0, 1);
	layout->addLayout(childLayout, 1, 1);
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
	config_save_safe(pluginConfig, ".ex.tmp", ".ex.back");
	dockWidget->setURL(hostLine->text().toStdString());
	dockWidget->reloadAds();
	this->close();
}

SettingsButton::SettingsButton()
{
	this->setText("&Settings");
	connect(this, &QPushButton::released, this,
		&SettingsButton::ButtonClicked);
}

AdControlWidget::AdControlWidget(std::string url)
{

	setURL(url);
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

void AdControlWidget::playAd()
{
	using namespace std::chrono_literals;
	adPlayButton->setEnabled(false);
	QNetworkAccessManager manager;
	std::string url;
	url.append(getURL().c_str()).append("/loadAd");
	std::string videoJSONstring =
		"{\"input\": \"" + url +
		"\",\"input_format\": \"mp4\", \"is_local_file\" : false, \"scale\": { \"x\": 1.5, \"y\": 1.5} }";
	obs_source *prevscene = obs_frontend_get_current_scene();
	obs_scene *adScene = obs_scene_create("Ad Scene");
	obs_source *scenesource = obs_scene_get_source(adScene);
	obs_data *mediasettings =
		obs_data_create_from_json(videoJSONstring.c_str());
	obs_source *adsource = obs_source_create("ffmpeg_source", "adSource",
						 mediasettings, nullptr);
	obs_scene_add(adScene, adsource);
	obs_frontend_set_current_scene(scenesource);

	//play ad
	std::this_thread::sleep_for(3s);
	//wait for ad to finish
	obs_frontend_set_current_scene(prevscene);
	obs_source_release(adsource);
	obs_source_remove(adsource);
	obs_scene_release(adScene);
	obs_source_remove(scenesource);
	obs_data_release(mediasettings);
	adPlayButton->setEnabled(true);
}

void AdControlWidget::getAds(std::string APIHost)
{
	obs_log(LOG_INFO, "getting Ads...");
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	std::string requestString;
	requestString.append(APIHost.c_str()).append("/getAds");
	QNetworkRequest *request = new QNetworkRequest(
		QUrl().fromPercentEncoding(requestString.c_str()));
	request->setRawHeader("User-Agent", "MyOwnBrowser 1.0");
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
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
#include <format>
#include <QLabel>
#include <iostream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <nlohmann/json.hpp>
#include <QLayout>

using json = nlohmann::json;
config_t *config;
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	/*config = obs_frontend_get_profile_config();
	if (config_open(&config, "/obs-ad-slice-controler.ini",
			CONFIG_OPEN_EXISTING) == CONFIG_FILENOTFOUND) {

		config = config_create("/obs-ad-slice-controler.ini");

		config_set_default_string(config, "API", "API-Host",
					  "http://localhost:5499");
		config_save_safe(config, ".ex.tmp", ".ex.back");
		obs_log(LOG_INFO, "Created new Adslice Config");
	} else if (config_open(&config, "/obs-ad-slice-controler.ini",
			       CONFIG_OPEN_EXISTING) == CONFIG_ERROR) {
		obs_log(LOG_INFO, "Error in config file - reseting...");
		config = config_create("/obs-ad-slice-controler.ini");
	}
	obs_log(LOG_INFO, "config found reading config");
	config_open(&config, "/obs-ad-slice-controler.ini",
		    CONFIG_OPEN_EXISTING);
	 	const char *confurl = config_get_string(config, "API", "API-Host");
	if (!confurl)
		confurl = "";
	obs_log(LOG_INFO, confurl); */

	char pluginID[] = "obs-ad_slice_controller_100";
	AdControlWidget *dockWidget = new AdControlWidget();
	dockWidget->setMinimumHeight(200);
	dockWidget->setMinimumWidth(150);
	dockWidget->setURL("http://localhost:5499");

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

AdControlWidget::AdControlWidget()
{
	vbox->setSpacing(0);
	this->vbox->addWidget(refresh, 0, Qt::AlignTop);
	this->vbox->addWidget(adSelection, 0, Qt::AlignTop);
	connect(adPlayButton, &QPushButton::released, this,
		&AdControlWidget::playAd);
	connect(refresh, &QPushButton::released, this,
		&AdControlWidget::reloadAds);
	this->vbox->addWidget(adPlayButton, 0, Qt::AlignTop);
	this->setLayout(vbox);
	reloadAds();
}

void AdControlWidget::playAd()
{

	adPlayButton->setEnabled(false);
	obs_source *prevscene = obs_frontend_get_current_scene();
	obs_scene *adScene = obs_scene_create("Ad Scene");
	obs_source *scenesource = obs_scene_get_source(adScene);
	obs_source *adsource =
		obs_source_create_private("ffmpeg_source", "adSource", NULL);

	obs_frontend_set_current_scene(scenesource);

	//play ad
	//wait for ad to finish
	obs_frontend_set_current_scene(prevscene);
	obs_source_release(adsource);
	obs_source_remove(adsource);
	obs_scene_release(adScene);
	obs_source_remove(scenesource);
	adPlayButton->setEnabled(true);
}

void AdControlWidget::getAds(std::string APIHost)
{
	obs_log(LOG_INFO, "getting Ads...");
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	obs_log(LOG_INFO, APIHost.c_str());
	QNetworkRequest *request = new QNetworkRequest(
		QUrl().fromPercentEncoding("http://localhost:5499/getAds"));
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

			} else // handle error
			{
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
	//get menu from API

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
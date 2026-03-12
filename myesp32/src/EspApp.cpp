/*
 * MIT License
 *
 * Copyright (c) 2026 Gary Huang (deepkh@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction...
 */
#include "EspApp.h"
#include "EspConfig.h"

MyEsp::WifiService* MyEsp::WifiService::instance_ = nullptr;
MyEsp::MqttService* MyEsp::MqttService::instance_ = nullptr;
MyEsp::NtpClientService* MyEsp::NtpClientService::instance_ = nullptr;
MyEsp::WebServerService* MyEsp::WebServerService::instance_ = nullptr;
MyEsp::OtaUpdaterService* MyEsp::OtaUpdaterService::instance_ = nullptr;
MyEsp::EspApp espApp(g_espconfig);

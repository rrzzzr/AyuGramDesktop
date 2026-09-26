/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "core/current_geo_location.h"

#include "base/platform/base_platform_info.h"
#include "base/invoke_queued.h"
#include "base/timer.h"
#include "data/raw/raw_countries_bounds.h"
#include "platform/platform_current_geo_location.h"
#include "ui/ui_utility.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QCoreApplication>
#include <QtCore/QPointer>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

namespace Core {
namespace {

void ResolveLocationAddressGeneric(
		const GeoLocation &location,
		const QString &language,
		const QString &token,
		Fn<void(GeoAddress)> callback) {
	callback(GeoAddress());
}

} // namespace

GeoLocation ResolveCurrentCountryLocation() {
	const auto iso2 = Platform::SystemCountry().toUpper();
	const auto &bounds = Raw::CountryBounds();
	const auto i = bounds.find(iso2);
	if (i == end(bounds)) {
		return {
			.accuracy = GeoLocationAccuracy::Failed,
		};
	}
	return {
		.point = {
			(i->second.minLat + i->second.maxLat) / 2.,
			(i->second.minLon + i->second.maxLon) / 2.,
		},
		.bounds = {
			i->second.minLat,
			i->second.minLon,
			i->second.maxLat - i->second.minLat,
			i->second.maxLon - i->second.minLon,
		},
		.accuracy = GeoLocationAccuracy::Country,
	};
}

void ResolveCurrentGeoLocation(Fn<void(GeoLocation)> callback) {
	using namespace Platform;
	return ResolveCurrentExactLocation([done = std::move(callback)](
			GeoLocation result) {
		done(result.accuracy != GeoLocationAccuracy::Failed
			? result
			: ResolveCurrentCountryLocation());
	});
}

void ResolveLocationAddress(
		const GeoLocation &location,
		const QString &language,
		const QString &token,
		Fn<void(GeoAddress)> callback) {
	auto done = [=, done = std::move(callback)](GeoAddress result) mutable {
		if (!result && !token.isEmpty()) {
			ResolveLocationAddressGeneric(
				location,
				language,
				token,
				std::move(done));
		} else {
			done(result);
		}
	};
	Platform::ResolveLocationAddress(location, language, std::move(done));
}

bool AreTheSame(const GeoLocation &a, const GeoLocation &b) {
	if (a.accuracy != GeoLocationAccuracy::Exact
		|| b.accuracy != GeoLocationAccuracy::Exact) {
		return false;
	}
	const auto normalize = [](float64 value) {
		value = std::fmod(value + 180., 360.);
		return (value + (value < 0. ? 360. : 0.)) - 180.;
	};
	constexpr auto kEpsilon = 0.0001;
	const auto lon1 = normalize(a.point.y());
	const auto lon2 = normalize(b.point.y());
	const auto diffLat = std::abs(a.point.x() - b.point.x());
	if (std::abs(a.point.x()) >= (90. - kEpsilon)
		|| std::abs(b.point.x()) >= (90. - kEpsilon)) {
		return diffLat <= kEpsilon;
	}
	auto diffLon = std::abs(lon1 - lon2);
	if (diffLon > 180.) {
		diffLon = 360. - diffLon;
	}

	return diffLat <= kEpsilon && diffLon <= kEpsilon;
}

} // namespace Core

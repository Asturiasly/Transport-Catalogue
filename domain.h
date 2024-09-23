#pragma once
#include <string>
#include <set>
#include <vector>

#include "geo.h"

namespace domain
{
	struct Geo : public Coordinates {};

	struct Stop
	{
		std::string name;
		Geo::Coordinates coordinates;
		std::set<std::string_view> buses_to_stop_ = {};
	};

	struct Bus
	{
		std::string name;
		std::vector<Stop*> route;
		bool is_circle;
	};

	struct BusInfo
	{
		size_t stops_count;
		size_t unique_stops_count;
		double total_metres_distance;
		double curvature;
	};

	namespace hasher
	{
		class StrView {
		public:
			size_t operator()(const std::string_view str) const;
			bool operator==(const std::string_view other) const
			{
				return *this == other;
			}
		private:
			std::hash<std::string_view> hasher_;
		};

		class StopsDistancePair
		{
		public:
			size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*>& points) const;
			bool operator==(const std::pair<const Stop*, const Stop*>& other) const
			{
				return *this == other;
			}
		private:
			std::hash<const void*> hasher_;
		};
	} //hasher
}

#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace reader
{
	class JSONReader
	{
	public:
		JSONReader(std::istream& input) : doc_(json::Load(input)) {}

		void FillCatalogue(catalogue::TransportCatalogue& catalogue);
		
		const json::Node& GetBaseRequests() const;
		const json::Node& GetStatRequests() const;
		const render::RenderSettings GetMapSettings() const;
		const router::RoutingSettings GetRoutingSettings() const;

	private:
		json::Document doc_;
		json::Node nothing_ = nullptr;
		static void AddDistancesToStop(catalogue::TransportCatalogue&, std::vector<json::Dict>&);
		static std::pair<std::vector<json::Dict>, std::vector<json::Dict>> FillStops(catalogue::TransportCatalogue&, json::Array&);
		static void FillBuses(catalogue::TransportCatalogue&, std::vector<json::Dict>&);
		const json::Node& GetRenderSettings() const;
		const json::Node& GetRouteSettings() const;
	};
}
#include <variant>
#include "json_reader.h"

const json::Node& reader::JSONReader::GetBaseRequests() const {
	if (!doc_.GetRoot().AsDict().count("base_requests"))
		return nothing_;
	return doc_.GetRoot().AsDict().at("base_requests");
}

const json::Node& reader::JSONReader::GetStatRequests() const {
	if (!doc_.GetRoot().AsDict().count("stat_requests"))
		return nothing_;
	return doc_.GetRoot().AsDict().at("stat_requests");
}

const json::Node& reader::JSONReader::GetRenderSettings() const
{
	if (!doc_.GetRoot().AsDict().count("render_settings"))
		return nothing_;
	return doc_.GetRoot().AsDict().at("render_settings");
}

const json::Node& reader::JSONReader::GetRouteSettings() const
{
	if (!doc_.GetRoot().AsDict().count("routing_settings"))
		return nothing_;
	return doc_.GetRoot().AsDict().at("routing_settings");
}

void reader::JSONReader::FillCatalogue(catalogue::TransportCatalogue& catalogue)
{
	auto base_requests_arr = GetBaseRequests().AsArray();
	std::pair<std::vector<json::Dict>, std::vector<json::Dict>> stops_and_buses_requests;
	stops_and_buses_requests = reader::JSONReader::FillStops(catalogue, base_requests_arr);
	reader::JSONReader::AddDistancesToStop(catalogue, stops_and_buses_requests.first);
	reader::JSONReader::FillBuses(catalogue, stops_and_buses_requests.second);
}

const render::RenderSettings reader::JSONReader::GetMapSettings() const
{
	render::RenderSettings result;
	auto render_settings = GetRenderSettings().AsDict();
	result.width = render_settings.at("width").AsDouble();
	result.height = render_settings.at("height").AsDouble();
	result.padding = render_settings.at("padding").AsDouble();
	result.line_width = render_settings.at("line_width").AsDouble();
	result.stop_radius = render_settings.at("stop_radius").AsDouble();
	result.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
	auto bus_label_offset = render_settings.at("bus_label_offset").AsArray();
	result.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
	result.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
	auto stop_label_offset = render_settings.at("stop_label_offset").AsArray();
	result.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };
	result.underlayer_width = render_settings.at("underlayer_width").AsDouble();

	if (render_settings.at("underlayer_color").IsString())
		result.underlayer_color = render_settings.at("underlayer_color").AsString();
	else
	{
		if (render_settings.at("underlayer_color").AsArray().size() == 3)
		{
			auto underlayer_color = render_settings.at("underlayer_color").AsArray();
			result.underlayer_color = svg::Rgb({ 
				static_cast<uint8_t>(underlayer_color[0].AsInt()), 
				static_cast<uint8_t>(underlayer_color[1].AsInt()),
				static_cast<uint8_t>(underlayer_color[2].AsInt()) });
		}
		else if (render_settings.at("underlayer_color").AsArray().size() == 4)
		{
			auto underlayer_color = render_settings.at("underlayer_color").AsArray();
			result.underlayer_color = svg::Rgba({ 
				static_cast<uint8_t>(underlayer_color[0].AsInt()),
				static_cast<uint8_t>(underlayer_color[1].AsInt()),
				static_cast<uint8_t>(underlayer_color[2].AsInt()),
				underlayer_color[3].AsDouble() });
		}
		else
		{
			throw std::logic_error("wrong color type");
		}
	}
	auto color_palette = render_settings.at("color_palette").AsArray();
	for (const auto& color : color_palette)
	{
		if (color.IsString())
			result.color_palette.push_back(color.AsString());
		else
		{
			if (color.AsArray().size() == 3)
			{
				result.color_palette.push_back(svg::Rgb({ 
					static_cast<uint8_t>(color.AsArray()[0].AsInt()),
					static_cast<uint8_t>(color.AsArray()[1].AsInt()),
					static_cast<uint8_t>(color.AsArray()[2].AsInt()) }));
			}
			else if (color.AsArray().size() == 4)
			{
				result.color_palette.push_back(svg::Rgba({ 
					static_cast<uint8_t>(color.AsArray()[0].AsInt()),
					static_cast<uint8_t>(color.AsArray()[1].AsInt()),
					static_cast<uint8_t>(color.AsArray()[2].AsInt()),
					color.AsArray()[3].AsDouble() }));
			}
			else
				throw std::logic_error("wrong color type");
		}
	}
	return result;
}

const router::RoutingSettings reader::JSONReader::GetRoutingSettings() const
{
	auto settings = GetRouteSettings().AsDict();
	router::RoutingSettings result;
	result.bus_velocity = settings.at("bus_velocity").AsDouble();
	result.bus_wait_time = settings.at("bus_wait_time").AsInt();
	return result;
}

void reader::JSONReader::AddDistancesToStop(catalogue::TransportCatalogue& catalogue, std::vector<json::Dict>& data)
{
	for (size_t i = 0; i < data.size(); ++i)
	{
		bool is_empty = false;
		const domain::Stop* from = catalogue.GetStop(data[i].at("name").AsString());
		json::Dict distances;
		try
		{
			distances = data[i].at("road_distances").AsDict();
		}
		catch (...)
		{
			is_empty = true;
		}

		for (const auto& [stopname, distance] : distances)
		{
			if (is_empty)
				break;
			const domain::Stop* to = catalogue.GetStop(stopname);
			int metres = distance.AsInt();
			catalogue.SetDistanceStops(from, to, metres);
		}
	}
}

void reader::JSONReader::FillBuses(catalogue::TransportCatalogue& catalogue, std::vector<json::Dict>& buses_requests)
{
	for (size_t i = 0; i < buses_requests.size(); ++i)
	{
		std::vector<std::string_view> route;
		if (buses_requests[i].at("is_roundtrip").AsBool())
		{
			for (size_t j = 0; j < buses_requests[i].at("stops").AsArray().size(); ++j)
			{
				route.push_back(buses_requests[i].at("stops").AsArray()[j].AsString());
			}
			catalogue.AddBus(buses_requests[i].at("name").AsString(), route, buses_requests[i].at("is_roundtrip").AsBool());
		}
		else
		{
			route.resize(buses_requests[i].at("stops").AsArray().size() * 2 - 1);
			for (size_t j = 0; j < buses_requests[i].at("stops").AsArray().size(); ++j)
			{
				route[j] = buses_requests[i].at("stops").AsArray()[j].AsString();
				if (j < route.size() / 2)
					route[route.size() - j - 1] = buses_requests[i].at("stops").AsArray()[j].AsString();
			}
			catalogue.AddBus(buses_requests[i].at("name").AsString(), route, buses_requests[i].at("is_roundtrip").AsBool());
		}
	}
}

std::pair<std::vector<json::Dict>, std::vector<json::Dict>> reader::JSONReader::FillStops(catalogue::TransportCatalogue& catalogue, json::Array& all_base_data)
{
	std::vector<json::Dict> stops_requests;
	std::vector<json::Dict> buses_requests;
	for (size_t i = 0; i < all_base_data.size(); ++i)
	{
		auto base_data = all_base_data[i].AsDict();
		if (base_data["type"].AsString() == "Stop")
		{
			catalogue.AddStop(base_data.at("name").AsString(), { base_data.at("latitude").AsDouble(), base_data.at("longitude").AsDouble() });
			stops_requests.push_back(std::move(base_data));
		}
		else
			buses_requests.push_back(std::move(base_data));
	}
	return { stops_requests, buses_requests };
}
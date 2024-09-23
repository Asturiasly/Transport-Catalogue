#include "request_handler.h"

const json::Node RequestHandler::JSONGetBusInfo(const json::Dict& request_map) const
{
	json::Node result;
	const std::string& route_number = request_map.at("name").AsString();
	const int id = request_map.at("id").AsInt();

	if (db_.GetBus(route_number) == nullptr) {
		result = json::Builder{}.
					StartDict().
						Key("error_message").Value("not found").
						Key("request_id").Value(id).
					 EndDict().
					 Build();
	}
	else {
		result = json::Builder{}.StartDict().
									Key("request_id").Value(id).
									Key("curvature").Value(GetBusInfo(route_number)->curvature).
									Key("route_length").Value(GetBusInfo(route_number)->total_metres_distance).
									Key("stop_count").Value(static_cast<int>(GetBusInfo(route_number)->stops_count)).
									Key("unique_stop_count").Value(static_cast<int>(GetBusInfo(route_number)->unique_stops_count)).
								EndDict().
								Build();
	}
	return result;
}

const json::Node RequestHandler::JSONGetStopInfo(const json::Dict& request_map) const
{
	json::Node result;
	const std::string& stopname = request_map.at("name").AsString();
	const int id = request_map.at("id").AsInt();

	if (db_.GetStop(stopname) == nullptr) {
		result = json::Builder{}.StartDict().
									Key("request_id").Value(id).
									Key("error_message").Value("not found").
								 EndDict().
								 Build();
	}
	else {
		json::Array buses;
		for (auto& bus : GetBusesToStop(stopname)) {
			buses.push_back(std::string(bus));
		}
		result = json::Builder{}.StartDict().
									Key("request_id").Value(id).
									Key("buses").Value(buses).
								 EndDict().
								 Build();
	}
	return result;
}

const json::Node RequestHandler::JSONGetMap(const json::Dict& request_map) const
{
	json::Node result;

	result = json::Builder{}.StartDict().
								Key("request_id").Value(request_map.at("id").AsInt()).
								Key("map").Value(PrintMap().str()).
							 EndDict().
							 Build();
	return result;
}

const json::Node RequestHandler::JSONGetRoute(const json::Dict& request_map) const
{
	json::Node result;
	const int id = request_map.at("id").AsInt();
	const auto from = request_map.at("from").AsString();
	const auto to = request_map.at("to").AsString();

	//std::string temp_1 = "Biryulyovo Tovarnaya";
	//std::string temp_2 = "Prazhskaya";

	auto route = router_.BuildRoute(router_.GetVertex(from) - 1, router_.GetVertex(to) - 1);

	using namespace std::string_literals;

	if (!route) {
		result = json::Builder{}
			.StartDict()
			.Key("request_id"s).Value(id)
			.Key("error_message"s).Value("not found"s)
			.EndDict()
			.Build();
		return result;
	}

	json::Array items;
	double total_time = 0;
	items.reserve(route.value().edges.size());

	for (auto& edge_id : route.value().edges) {
		const graph::Edge<double> edge = router_.GetGraph().GetEdge(edge_id);
		if (edge.span_count == 0) {
			items.emplace_back(json::Node(json::Builder{}
				.StartDict()
				.Key("stop_name"s).Value(edge.stop)
				.Key("time"s).Value(edge.weight)
				.Key("type"s).Value("Wait"s)
				.EndDict()
				.Build()));

			total_time += edge.weight;
		}
		else
		{
			items.emplace_back(json::Node(json::Builder{}
				.StartDict()
				.Key("bus"s).Value(edge.stop)
				.Key("span_count"s).Value(edge.span_count)
				.Key("time"s).Value(edge.weight)
				.Key("type"s).Value("Bus"s).EndDict().Build()));
			total_time += edge.weight;
		}
	}


	result = json::Builder{}
		.StartDict()
		.Key("request_id"s).Value(id)
		.Key("total_time"s).Value(total_time)
		.Key("items"s).Value(items)
		.EndDict()
		.Build();

	return result;
}

void RequestHandler::PrintJSON(std::ostream& out) const
{
	json::Array result;
	const json::Array& arr = requests_.GetStatRequests().AsArray();
	for (auto& request : arr) {
		const auto& request_map = request.AsDict();
		const auto& type = request_map.at("type").AsString();
		if (type == "Stop") 
		{
			result.push_back(JSONGetStopInfo(request_map).AsDict());
		}
		if (type == "Bus") 
		{
			result.push_back(JSONGetBusInfo(request_map).AsDict());
		}
		if (type == "Map")
		{
			result.push_back(JSONGetMap(request_map).AsDict());
		}
		if (type == "Route")
		{
			result.push_back(JSONGetRoute(request_map).AsDict());
		}
	}
	json::Print(json::Document{ result }, out);
}

std::ostringstream RequestHandler::PrintMap() const
{
	std::ostringstream stream;
	auto map = renderer_.Rendering(db_.GetSortedBuses(), db_.GetSortedStops());
	map.Render(stream);
	return stream;
}

std::optional<domain::BusInfo> RequestHandler::GetBusInfo(std::string_view busname) const
{
	std::optional<domain::BusInfo> result;
	if (db_.GetBus(busname) != nullptr)
	{
		auto busname_to_bus = db_.GetBus(busname);
		size_t stops_count = busname_to_bus->route.size();
		std::unordered_set<domain::Stop*> unique_stops;
		double curvature = 0.0;
		double total_geo_distance = 0.0;
		double total_metres_distance = 0;
		domain::Geo::Coordinates from = { 0.0, 0.0 };
		domain::Geo::Coordinates to = { 0.0, 0.0 };

		for (size_t i = 0; i < busname_to_bus->route.size(); ++i)
		{
			unique_stops.insert(busname_to_bus->route[i]);
			from = busname_to_bus->route[i]->coordinates;
			if (i + 1 < busname_to_bus->route.size())
			{
				to = busname_to_bus->route[i + 1]->coordinates;
				total_geo_distance += domain::Geo::ComputeDistance(from, to);
				if (db_.GetDistanceStops(busname_to_bus->route[i], busname_to_bus->route[i + 1]))
					total_metres_distance += db_.GetDistanceStops(busname_to_bus->route[i], busname_to_bus->route[i + 1]);
				else
					total_metres_distance += db_.GetDistanceStops(busname_to_bus->route[i + 1], busname_to_bus->route[i]);
			}
		}

		curvature = total_metres_distance / total_geo_distance;
		size_t unique_stops_count = unique_stops.size();
		result = { std::move(stops_count), std::move(unique_stops_count), std::move(total_metres_distance), std::move(curvature) };
	}

	return result;
}

std::vector <std::string_view> RequestHandler::GetBusesToStop(std::string_view stopname) const
{
	std::vector <std::string_view> result;
	if (db_.GetStop(std::string(stopname)) != nullptr)
	{
		const domain::Stop* stopname_to_stop = db_.GetStop(std::string(stopname));
		if (stopname_to_stop->buses_to_stop_.empty())
			return result;
		else
		{
			for (auto& bus : stopname_to_stop->buses_to_stop_)
			{
				result.push_back(bus);
			}
		}
	}
	else
		result.push_back("not found");

	return result;
}

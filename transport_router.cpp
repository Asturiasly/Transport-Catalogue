#include "transport_router.h"
#include <iostream>

const graph::DirectedWeightedGraph<double>& router::TransportRouter::BuildGraph()
{
	auto buses = db_.GetAllBuses();
	auto stops = db_.GetAllStops();
	graph::DirectedWeightedGraph<double> result(stops.size() * 2);

	MakeStopsEdges(stops, result); // Метод для создания графа. Проводит ребра между вершиной самой остановки и вершиной ожидания остановки. Всего вершин "кол-во остановок" * 2

	for (const auto& bus : buses)
	{
		
		if (bus.second->is_circle)
		{
			MakeCircleRouteEdges(bus, result);
		}
		else
		{
			MakeNonCircleRouteEdges(bus, result);
		}
	}

	graph_ = std::move(result);

	return graph_;
}

std::optional<graph::Router<double>::RouteInfo> router::TransportRouter::BuildRoute(graph::VertexId from, graph::VertexId to)
{
	return router_.BuildRoute(from, to);
}

graph::VertexId router::TransportRouter::GetVertex(std::string name) const
{
	if (stops_id_.count(name))
		return stops_id_.at(name);
	throw std::logic_error("name does not exist");
}

router::RoutingSettings router::TransportRouter::GetSettings() const
{
	return settings_;
}

const graph::DirectedWeightedGraph<double>& router::TransportRouter::GetGraph() const
{
	return graph_;
}


void router::TransportRouter::MakeStopsEdges(const std::unordered_map<std::string_view, domain::Stop*, domain::hasher::StrView>& stops, graph::DirectedWeightedGraph<double>& result)
{
	graph::VertexId vertex = 0;
	int counter = 0;
	for (const auto& stop : stops)
	{
		result.AddEdge({ vertex++, vertex, static_cast<double>(settings_.bus_wait_time), 0, stop.second->name });
		stops_id_[stop.second->name] = vertex;

		if (counter == stops.size() - 1)
		{
			break;
		}  

		++vertex;
		++counter;
	}
}

void router::TransportRouter::MakeCircleRouteEdges(const std::pair<const std::string_view, domain::Bus*>& bus, graph::DirectedWeightedGraph<double>& result)
{
	const double speed = settings_.bus_velocity * 1000 / 60; // km/h -> metres/minute
	auto route = bus.second->route;
	for (size_t i = 0; i < route.size() - 1; ++i)
	{
		int span_count = 1;
		int distance = 0;
		for (size_t j = i; j < route.size(); ++j)
		{
			if (j + 1 < route.size())
			{
				distance += db_.GetDistanceStops(route[j], route[j + 1]);
				result.AddEdge({ stops_id_[route[i]->name], stops_id_[route[j + 1]->name] - 1, distance / speed, span_count++, bus.second->name });
			}
		}
	}
}
void router::TransportRouter::MakeNonCircleRouteEdges(const std::pair<const std::string_view, domain::Bus*>& bus, graph::DirectedWeightedGraph<double>& result)
{
	const double speed = settings_.bus_velocity * 1000 / 60; // km/h -> metres/minute
	auto route = bus.second->route;
	for (size_t i = 0; i < route.size() / 2; ++i)
	{
		int span_count = 1;
		int distance = 0;
		for (size_t j = i; j < route.size() / 2; ++j)
		{
			if (j < route.size() / 2)
			{
				if (db_.GetDistanceStops(route[j], route[j + 1]) == 0)
					distance += db_.GetDistanceStops(route[j + 1], route[j]);
				else
					distance += db_.GetDistanceStops(route[j], route[j + 1]);
			}
			result.AddEdge({ stops_id_[route[i]->name], stops_id_[route[j + 1]->name] - 1, distance / speed, span_count++, bus.second->name });
		}
	}

	for (size_t i = route.size() / 2; i < route.size(); ++i)
	{
		int span_count = 1;
		int reverse_distance = 0;
		for (size_t j = i; j < route.size(); ++j)
		{

			if (j + 1 < route.size())
			{
				if (db_.GetDistanceStops(route[j], route[j + 1]) == 0)
					reverse_distance += db_.GetDistanceStops(route[j + 1], route[j]);
				else
					reverse_distance += db_.GetDistanceStops(route[j], route[j + 1]);

				result.AddEdge({ stops_id_[route[i]->name], stops_id_[route[j + 1]->name] - 1, reverse_distance / speed, span_count++, bus.second->name });
			}
		}
	}
}
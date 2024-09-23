#pragma once
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"


namespace router
{
	struct RoutingSettings
	{
		int bus_wait_time;
		double bus_velocity;
	};

	class TransportRouter
	{
	public:
		TransportRouter(RoutingSettings settings, catalogue::TransportCatalogue& db) : settings_(std::move(settings)), db_(db), router_(BuildGraph())
		{}

		std::optional<graph::Router<double>::RouteInfo> BuildRoute(graph::VertexId from, graph::VertexId to);

		graph::VertexId GetVertex(std::string) const;



		RoutingSettings GetSettings() const;

		const graph::DirectedWeightedGraph<double>& GetGraph() const;

	private:
		RoutingSettings settings_;
		catalogue::TransportCatalogue& db_;

		const graph::DirectedWeightedGraph<double>& BuildGraph();
		graph::DirectedWeightedGraph<double> graph_;
		std::unordered_map<std::string, graph::VertexId> stops_id_;
		graph::Router<double> router_;

		void MakeStopsEdges(const std::unordered_map<std::string_view, domain::Stop*, domain::hasher::StrView>&, graph::DirectedWeightedGraph<double>&); // Метод для создания графа. Проводит ребра между вершиной самой остановки и вершиной ожидания остановки. Всего вершин "кол-во остановок" * 2
		void MakeCircleRouteEdges(const std::pair<const std::string_view, domain::Bus*>&, graph::DirectedWeightedGraph<double>&); // Заполняем ребра в кольцевых маршрутах
		void MakeNonCircleRouteEdges(const std::pair<const std::string_view, domain::Bus*>&, graph::DirectedWeightedGraph<double>&); //Заполняем ребра в некольцевых маршрутах
	};
}
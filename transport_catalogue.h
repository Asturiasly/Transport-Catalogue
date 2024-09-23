#pragma once

#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <iomanip>

#include "domain.h"

namespace catalogue
{
	class TransportCatalogue {
	public:
		void AddStop(std::string_view, domain::Geo::Coordinates); //добавляем остановку
		void AddBus(std::string_view, std::vector<std::string_view>&, bool); //добавляем автобус и его маршрут
		void SetDistanceStops(const domain::Stop*, const domain::Stop*, int); // добавляем дистанцию каждой остановке после добавления всех остановок
		const domain::Stop* GetStop(std::string) const; //получаем указатель на остановку по ключу названия остановки
		const domain::Bus* GetBus(std::string_view) const; //получаем указатель на автобус по ключу названия автобуса
		int GetDistanceStops(const domain::Stop*, const domain::Stop*) const; //получаем дистанцию между двумя остановками
		const std::map<std::string_view, const domain::Bus*> GetSortedBuses() const; //возвращаем отсортированный по названию маршрута контейнер
		const std::map<std::string_view, const domain::Stop*> GetSortedStops() const; //возвращаем отсртированные по названию остановки
		const std::unordered_map<std::string_view, domain::Stop*, domain::hasher::StrView> GetAllStops() const;
		const std::unordered_map<std::string_view, domain::Bus*, domain::hasher::StrView > GetAllBuses() const;
		const std::set<std::string_view> GetBusesToStop(std::string) const;

	private:
		std::deque<domain::Stop> stops_; //храним все остановки
		std::deque<domain::Bus> buses_;	//храним все автобусы
		std::unordered_map<std::string_view, domain::Stop*, domain::hasher::StrView> stopname_to_stop_; //храним указатели на остановки для быстрого поиска, Key - имя остановки
		std::unordered_map<std::string_view, domain::Bus*, domain::hasher::StrView> busname_to_bus_; //храним указатели на автобусы для быстрого поиска, Key - имя автобуса
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, domain::hasher::StopsDistancePair> distance_stops_; //храним расстояние от остановки до остановки, включая 2 одинаковые остановки
	};
} //namespace catalogue
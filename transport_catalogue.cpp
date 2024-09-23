#include "transport_catalogue.h"

void catalogue::TransportCatalogue::AddStop(std::string_view name, domain::Geo::Coordinates coords)
{
	stops_.push_back({ std::string(name), std::move(coords) });
	domain::Stop* cur_elem = &stops_[stops_.size() - 1];
	stopname_to_stop_[cur_elem->name] = std::move(cur_elem);
}

void catalogue::TransportCatalogue::AddBus(std::string_view name, std::vector<std::string_view>& str_route, bool is_circle)
{
	std::vector<domain::Stop*> route;

	for (size_t i = 0; i < str_route.size(); ++i)
	{
		route.push_back(stopname_to_stop_[str_route[i]]);
	}

	buses_.push_back({ std::string(name), std::move(route), is_circle });
	domain::Bus* cur_elem = &buses_[buses_.size() - 1];
	for (size_t i = 0; i < str_route.size(); ++i)
	{
		stopname_to_stop_[str_route[i]]->buses_to_stop_.insert(cur_elem->name);
	}
	busname_to_bus_[cur_elem->name] = std::move(cur_elem);
}

void catalogue::TransportCatalogue::SetDistanceStops(const domain::Stop* from, const domain::Stop* to, int distance)
{
	distance_stops_[{from, to}] = distance;
}

const std::map<std::string_view, const domain::Bus*> catalogue::TransportCatalogue::GetSortedBuses() const 
{
	std::map <std::string_view, const domain::Bus*> result;
	for (const auto& bus : busname_to_bus_) {
		result.insert(bus);
	}
	return result;
}

const std::map<std::string_view, const domain::Stop*> catalogue::TransportCatalogue::GetSortedStops() const 
{
	std::map <std::string_view, const domain::Stop*> result;
	for (const auto& stop : stopname_to_stop_) {
		result.insert(stop);
	}
	return result;
}

const domain::Stop* catalogue::TransportCatalogue::GetStop(std::string stopname) const
{
	if (stopname_to_stop_.count(stopname))
		return stopname_to_stop_.at(stopname);
	return nullptr;
}

const domain::Bus* catalogue::TransportCatalogue::GetBus(std::string_view busname) const
{
	if (busname_to_bus_.count(busname))
		return busname_to_bus_.at(busname);
	return nullptr;
}

int catalogue::TransportCatalogue::GetDistanceStops(const domain::Stop* from, const domain::Stop* to) const
{
	if (distance_stops_.count({ from, to }))
		return distance_stops_.at({ from, to });
	return 0;
}

const std::set<std::string_view> catalogue::TransportCatalogue::GetBusesToStop(std::string name) const
{
	if (stopname_to_stop_.count(name))
		return stopname_to_stop_.at(name)->buses_to_stop_;
	else
		throw std::logic_error("Stop does not exist");
}

const std::unordered_map<std::string_view, domain::Stop*, domain::hasher::StrView> catalogue::TransportCatalogue::GetAllStops() const
{
	return stopname_to_stop_;
}
const std::unordered_map<std::string_view, domain::Bus*, domain::hasher::StrView > catalogue::TransportCatalogue::GetAllBuses() const
{
	return busname_to_bus_;
}
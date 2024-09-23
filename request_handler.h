#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"

#include <sstream>
class RequestHandler
{
public:
    RequestHandler(router::TransportRouter& router, render::MapRenderer& renderer, catalogue::TransportCatalogue& db, reader::JSONReader& requests) : 
        router_(router), renderer_(renderer), db_(db), requests_(requests)  {}

    void PrintJSON(std::ostream& out) const;
    std::optional<domain::BusInfo> GetBusInfo(std::string_view) const; // получаем информацию о маршруте автобуса
    std::vector <std::string_view> GetBusesToStop(std::string_view) const; //получаем информация об автобусах, следующих через остановку

private:
    router::TransportRouter& router_;
    render::MapRenderer& renderer_;
    catalogue::TransportCatalogue& db_;
    reader::JSONReader& requests_;

    const json::Node JSONGetBusInfo(const json::Dict& request_map) const;
    const json::Node JSONGetStopInfo(const json::Dict& request_map) const;
    const json::Node JSONGetMap(const json::Dict& request_map) const;
    const json::Node JSONGetRoute(const json::Dict& request_map) const;
    std::ostringstream PrintMap() const;
};

#include "map_renderer.h"

svg::Point render::SphereProjector::operator()(Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Document render::MapRenderer::Rendering(const std::map<std::string_view, const domain::Bus*>& buses, const std::map<std::string_view, const domain::Stop*>& stops) const
{
    svg::Document result;
    std::vector<domain::Geo::Coordinates> route_stops_coord;
    for (const auto& [bus_number, bus] : buses) {
        for (const auto& stop : bus->route) {
            route_stops_coord.push_back(stop->coordinates);
        }
    }
    SphereProjector points(route_stops_coord.begin(), route_stops_coord.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
    for (const auto& line : GetLines(buses, points)) {
        result.Add(line);
    }
    for (const auto& route_name : GetRouteNames(buses, points)) {
        result.Add(route_name);
    }
    for (const auto& stop_symbol : GetStopsSymbols(stops, points))
    {
        result.Add(stop_symbol);
    }
    for (const auto& stopname : GetStopsNames(stops, points))
    {
        result.Add(stopname);
    }

    return result;
}

std::vector<svg::Polyline> render::MapRenderer::GetLines(const std::map<std::string_view, const domain::Bus*>& buses, const SphereProjector& points) const
{
    std::vector<svg::Polyline> result;

    int color_variations = 0;
    for (const auto& bus : buses)
    {
        if (bus.second->route.empty())
            continue;

        svg::Polyline line;
        for (size_t i = 0; i < bus.second->route.size(); ++i)
        {
            line.AddPoint(points(bus.second->route[i]->coordinates));
        }
        line.SetFillColor("none");
        line.SetStrokeWidth(render_settings_.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        line.SetStrokeColor(render_settings_.color_palette[color_variations++]);

        if (color_variations == static_cast<int>(render_settings_.color_palette.size()))
            color_variations = 0;

        result.push_back(line);
    }
    return result;
}

std::vector<svg::Text> render::MapRenderer::GetRouteNames(const std::map<std::string_view, const domain::Bus*>& buses, const SphereProjector& points) const
{
    std::vector<svg::Text> result;
    int color_variations = 0;
    const int non_circle_last_stops_size = 2;
    for (const auto& bus : buses)
    {
        if (bus.second->route.empty())
            continue;

        svg::Text underlayer;
        svg::Text route_name;

        underlayer.SetOffset(render_settings_.bus_label_offset);
        underlayer.SetFontSize(render_settings_.bus_label_font_size);
        underlayer.SetFontFamily("Verdana");
        underlayer.SetFontWeight("bold");
        underlayer.SetData(bus.second->name);
        underlayer.SetFillColor(render_settings_.underlayer_color);
        underlayer.SetStrokeColor(render_settings_.underlayer_color);
        underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        route_name.SetOffset(render_settings_.bus_label_offset);
        route_name.SetFontSize(render_settings_.bus_label_font_size);
        route_name.SetFontFamily("Verdana");
        route_name.SetFontWeight("bold");
        route_name.SetData(bus.second->name);
        route_name.SetFillColor(render_settings_.color_palette[color_variations++]);

        if (bus.second->is_circle || bus.second->route[0] == bus.second->route[bus.second->route.size() / 2])
        {
            underlayer.SetPosition(points(bus.second->route[0]->coordinates));
            route_name.SetPosition(points(bus.second->route[0]->coordinates));

            result.push_back(std::move(underlayer));
            result.push_back(std::move(route_name));
        }
        else
        {
            int first_and_midlle_stop = 0;
            for (int i = 0; i < non_circle_last_stops_size; ++i)
            {

                underlayer.SetPosition(points(bus.second->route[first_and_midlle_stop]->coordinates));
                route_name.SetPosition(points(bus.second->route[first_and_midlle_stop]->coordinates));

                first_and_midlle_stop = static_cast<int>(bus.second->route.size() / 2);
                result.push_back(underlayer);
                result.push_back(route_name);
            }
        }

        if (color_variations == static_cast<int>(render_settings_.color_palette.size()))
            color_variations = 0;
    }
    return result;
}

std::vector<svg::Circle> render::MapRenderer::GetStopsSymbols(const std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& points) const
{
    std::vector<svg::Circle> result;
    for (const auto& [stop_name, stop] : stops)
    {
        if (stop->buses_to_stop_.empty())
            continue;

        svg::Circle symbol;
        symbol.SetCenter(points(stop->coordinates));
        symbol.SetRadius(render_settings_.stop_radius);
        symbol.SetFillColor("white");

        result.push_back(symbol);
    }
    return result;
}

std::vector<svg::Text> render::MapRenderer::GetStopsNames(const std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& points) const
{
    std::vector<svg::Text> result;

    svg::Text underlayer;
    svg::Text route_name;

    for (const auto& [stop_name, stop] : stops) {
        if (stop->buses_to_stop_.empty())
            continue;

        underlayer.SetPosition(points(stop->coordinates));
        underlayer.SetOffset(render_settings_.stop_label_offset);
        underlayer.SetFillColor(render_settings_.underlayer_color);
        underlayer.SetStrokeColor(render_settings_.underlayer_color);
        underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        underlayer.SetFontSize(render_settings_.stop_label_font_size);
        underlayer.SetFontFamily("Verdana");
        underlayer.SetData(stop->name);

        route_name.SetFillColor("black");
        route_name.SetPosition(points(stop->coordinates));
        route_name.SetOffset(render_settings_.stop_label_offset);
        route_name.SetFontSize(render_settings_.stop_label_font_size);
        route_name.SetFontFamily("Verdana");
        route_name.SetData(stop->name);

        result.push_back(underlayer);
        result.push_back(route_name);
    }
    return result;
}
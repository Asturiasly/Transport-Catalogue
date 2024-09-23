#include <iostream>
#include <string>
#include <iomanip>
#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>
#include <fstream>

#include "map_renderer.h"
#include "request_handler.h"
#include "json_reader.h"
#include "transport_router.h"


int main()
{
	catalogue::TransportCatalogue catalogue;
	reader::JSONReader input(std::cin);
	input.FillCatalogue(catalogue);
	render::MapRenderer map(input.GetMapSettings());
	router::TransportRouter router(input.GetRoutingSettings(), catalogue);
	RequestHandler output(router, map, catalogue, input);
	output.PrintJSON(std::cout);
}
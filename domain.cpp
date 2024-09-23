#include "domain.h"

size_t domain::hasher::StrView::operator()(const std::string_view str) const
{
	return hasher_(str);
}

size_t domain::hasher::StopsDistancePair::operator()(const std::pair<const domain::Stop*, const domain::Stop*>& points) const
{
	size_t hash_first = hasher_(points.first);
	size_t hash_second = hasher_(points.second);
	return hash_first + hash_second * 37;
}
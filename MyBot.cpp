#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"

// functions
hlt::Planet FindClosestPlanet(const hlt::Ship& ship, std::vector<hlt::Planet> planets);
bool ShipIsMine(hlt::EntityId id, std::vector<hlt::Ship> ships);
const hlt::Ship& ShipFromId(hlt::EntityId id, std::vector<hlt::Ship> ships);

int main() {
    const hlt::Metadata metadata = hlt::initialize("SkyGuy");
    const hlt::PlayerId player_id = metadata.player_id;

    std::vector<hlt::Move> moves;
    for (;;) {
        moves.clear();
        const hlt::Map map = hlt::in::get_map(metadata.map_width, metadata.map_height);

        for (const hlt::Ship& ship : map.ships.at(player_id)) {
            if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
                continue;
            }
			
			if (map.planets.size() > 0){
				const hlt::Planet& target_planet = FindClosestPlanet(ship, map.planets);
				
				if (ship.can_dock(target_planet)){
					// dock on planet if able
					moves.push_back(hlt::Move::dock(ship.entity_id, target_planet.entity_id));
				} else {
					// check if i own this planet
					if (target_planet.owned){
						if (!ShipIsMine(target_planet.docked_ships[0], map.ships.at(player_id))){
							// someone else owns it
							// attack their ships
							// move towards planet
							const hlt::possibly<hlt::Move> move =
									hlt::navigation::navigate_ship_towards_target(map, ship, ShipFromId(target_planet.docked_ships[0], map.ships.at(player_id)).location, hlt::constants::MAX_SPEED, true, 10, 3.14/4);
							if (move.second) {
								moves.push_back(move.first);
							}
						}
					} else {
						// move towards planet
						const hlt::possibly<hlt::Move> move =
								hlt::navigation::navigate_ship_to_dock(map, ship, target_planet, hlt::constants::MAX_SPEED);
						if (move.second) {
							moves.push_back(move.first);
						}
					}
				}
			} else {
				// TODO -- something
			}
        }

        if (!hlt::out::send_moves(moves)) {
            hlt::Log::log("send_moves failed; exiting");
            break;
        }
    }
}

hlt::Planet FindClosestPlanet(const hlt::Ship& ship, std::vector<hlt::Planet> planets){
	// find closest planet
	hlt::Planet current_planet = planets[0];
	double shortest = 100000000;
	
	for (const hlt::Planet& planet : planets) {
		if (!planet.is_full()){
			double dist = ship.location.get_distance_to(planet.location);
			if (dist < shortest){
				current_planet = planet;
				shortest = dist;
			}
		}
	}
	
	return current_planet;
}

bool ShipIsMine(hlt::EntityId id, std::vector<hlt::Ship> ships){
	for (hlt::Ship s : ships){
		if (s.entity_id == id){
			return true;
		}
	}
	
	return false;
}

const hlt::Ship& ShipFromId(hlt::EntityId id, std::vector<hlt::Ship> ships){
	for (hlt::Ship s : ships){
		if (s.entity_id == id){
			return s;
		}
	}	
	
	return ships[0];
}






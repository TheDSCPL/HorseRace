#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

namespace Constants {
	const static int DEFAULT_CREDITS = 0;
	const static int ALL = -1;
	const static int ACTIVE = -2;
	const static int INACTIVE = -3;
	const static int LOGGED_IN = -4;
	const static int LOGGED_OFF = -5;
	const static int ADMIN = -6;
	const static int IN_SERVER = -7;

	enum class ClientType {
		LOGGED_OUT,
		REGULAR_CLIENT,
		ADMIN_CLIENT
	};

    const static std::string defaultClientErrorWhatMessage = "Something went wrong!";
    const static bool DEFAULT_ADMIN = false;

    const static int MAX_HORSES_ON_RACE = 10;
    const static int MIN_HORSES_ON_RACE = 3;
}

namespace SQLPreparedStatementsNames {
    //name, username, password, admin, credits
    const static std::string registerNewUser = "REGISTER_NEW_USER";
    //login_name
    const static std::string getUserId = "GET_USER_ID";
    //username, hashed_pass
    const static std::string checkUserAndPass = "CHECK_USER_AND_PASS";
    //user_id
    const static std::string checkAdmin = "CHECK_ADMIN";
    //user_id, new_pass, old_pass
    const static std::string changePassword = "CHANGE_PASSWORD";
    //user_id, admin
    const static std::string changeAdmin = "CHANGE_ADMIN";
    //horse_name
    const static std::string getHorseId = "GET_HORSE_ID";
    //horse_name, speed
    const static std::string insertHorse = "INSERT_HORSE";
    //horse_id, race_id, state
    const static std::string insertHorseInRace = "INSERT_HORSE_IN_RACE";
    //laps
    const static std::string insertRace = "INSERT_RACE";
    //horse_id, race_id
    const static std::string checkHorseOnRace = "CHECK_HORSE_ON_RACE";
    //
    const static std::string getLatestRaceId = "GET_LATEST_RACE_ID";
    //race_id
    const static std::string checkRaceExists = "CHECK_RACE_EXISTS";
    //horse_id
    const static std::string checkHorseExists = "CHECK_HORSE_EXISTS";
    //user_id
    const static std::string checkUserExists = "CHECK_USER_EXISTS";
    //user_id
	const static std::string getUserCredits = "GET_USER_CREDITS";
    //race_id
    const static std::string getRaceDate = "GET_RACE_DATE";
    //race_id
    const static std::string getNumHorsesOnRace = "GET_NUM_HORSES_ON_RACE";
    //race_id
    const static std::string getHorsesOnRace = "GET_HORSES_ON_RACE";
    //user_id, horse_id, race_id
    const static std::string checkBetExists = "CHECK_BET_EXISTS";
    //user_id, credits delta
    const static std::string addCredits = "ADD_CREDITS";
    //race_id
    const static std::string checkRaceStarted = "CHECK_RACE_STARTED";
    //user_id, horse_id, race_id, new_bet
    const static std::string changeBet = "CHANGE_BET";
    //
    const static std::string getAllRaces = "GET_ALL_RACES";
    //race_id
    const static std::string hasRaceStarted = "HAS_RACE_STARTED";
    //race_id
    const static std::string getRaceLaps = "GET_RACE_LAPS";
    //maxNumberOfHorses (if 0 or negative, shows 5 horses)
    const static std::string getHorseRanks = "GET_HORSE_RANKS";
    //user_id, maxNumberOfHorses
    const static std::string getBetsPerUser = "GET_BETS_PER_USER";
    //
    const static std::string getAllUsers = "GET_ALL_USERS";
    //
    const static std::string getLoggedInUsers_dropTempTableIfExists = "GET_LOGGED_IN_USERS__DROP_TEMP_TABLE";
    //
    const static std::string getLoggedInUsers_createTempTable = "GET_LOGGED_IN_USERS__CREATE_TEMP_TABLE";
    //user_id, (socket OR "'SERVER'" if logged in the server)
    const static std::string getLoggedInUsers_insertIntoTempTable = "GET_LOGGED_IN_USERS__INSERT_INTO_TEMP_TABLE";
    //
    const static std::string getLoggedInUsers_getLoggedInUsers = "GET_LOGGED_IN_USERS__GET_LOGGED_IN_USERS";
    //
    const static std::string getLoggedInUsers_destroyTempTable = "GET_LOGGED_IN_USERS__DESTROY_TEMP_TABLE";
}

#endif
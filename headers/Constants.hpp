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
}

namespace SQLPreparedStatementsNames {
	const static std::string registerNewUser = "REGISTER_NEW_USER";
	const static std::string getUserId = "GET_USER_ID";
	const static std::string checkUserAndPass = "CHECK_USER_AND_PASS";
	const static std::string changePassword = "CHANGE_PASSWORD";
	const static std::string changeAdmin = "CHANGE_ADMIN";
	const static std::string getHorseId = "GET_HORSE_ID";
	const static std::string insertHorse = "INSERT_HORSE";
	const static std::string insertHorseInRace = "INSERT_HORSE_IN_RACE";
	const static std::string insertRace = "INSERT_RACE";
	const static std::string getLatestRaceId = "GET_LATEST_RACE_ID";
	const static std::string checkRaceExists = "CHECK_RACE_EXISTS";
	const static std::string checkUserExists = "CHECK_USER_EXISTS";
	const static std::string checkHorseExists = "CHECK_HORSE_EXISTS";
	const static std::string getUserCredits = "GET_USER_CREDITS";
	const static std::string getRaceDate = "GET_RACE_DATE";
	const static std::string getNumHorsesOnRace = "GET_NUM_HORSES_ON_RACE";
	const static std::string getHorsesOnRace = "GET_HORSES_ON_RACE";
	const static std::string checkBetExists = "CHECK_BET_EXISTS";
	const static std::string addCredits = "ADD_CREDITS";
	const static std::string checkRaceStarted = "CHECK_RACE_STARTED";
	const static std::string changeBet = "CHANGE_BET";
	const static std::string getAllRaces = "GET_ALL_RACES";
	const static std::string hasRaceStarted = "HAS_RACE_STARTED";
	const static std::string getRaceLaps = "GET_RACE_LAPS";
	const static std::string getHorseRanks = "GET_HORSE_RANKS";
	const static std::string getBetsPerUser = "GET_BETS_PER_USER";
	const static std::string getAllUsers = "GET_ALL_USERS";
	const static std::string getLoggedInUsers_createTempTable = "GET_LOGGED_IN_USERS__CREATE_TEMP_TABLE";
	const static std::string getLoggedInUsers_insertIntoTempTable = "GET_LOGGED_IN_USERS__INSERT_INTO_TEMP_TABLE";
	const static std::string getLoggedInUsers_getLoggedInUsers = "GET_LOGGED_IN_USERS__GET_LOGGED_IN_USERS";
	const static std::string getLoggedInUsers_destroyTempTable = "GET_LOGGED_IN_USERS__DESTROY_TEMP_TABLE";
}

#endif
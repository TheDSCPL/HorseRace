--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

SET search_path = public, pg_catalog;

--
-- Name: num_horses_race(integer); Type: FUNCTION; Schema: public; Owner: bop2
--

CREATE FUNCTION num_horses_race(r_i integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$
DECLARE t INTEGER;
BEGIN
        SELECT  COUNT(horse_id) INTO t
        FROM    are_on
        WHERE   race_id = r_i;

        RETURN t;
END;
$$;


ALTER FUNCTION public.num_horses_race(r_i integer) OWNER TO bop2;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: are_on; Type: TABLE; Schema: public; Owner: bop2; Tablespace: 
--

CREATE TABLE are_on (
    horse_id integer NOT NULL,
    race_id integer NOT NULL,
    state numeric DEFAULT 0 NOT NULL,
    place integer,
    CONSTRAINT is_on_check_max_horses_per_race CHECK ((num_horses_race(race_id) <= 10)),
    CONSTRAINT is_on_place_check CHECK (((place >= 1) OR (place = NULL::integer))),
    CONSTRAINT is_on_state_check CHECK (((state >= (0)::numeric) AND (state <= (1)::numeric)))
);


ALTER TABLE are_on OWNER TO bop2;

--
-- Name: TABLE are_on; Type: COMMENT; Schema: public; Owner: bop2
--

COMMENT ON TABLE are_on IS 'contains the horses that raced in each race';


--
-- Name: COLUMN are_on.state; Type: COMMENT; Schema: public; Owner: bop2
--

COMMENT ON COLUMN are_on.state IS 'horse''s state in the race (to multiply by its maximum speed)';


--
-- Name: COLUMN are_on.place; Type: COMMENT; Schema: public; Owner: bop2
--

COMMENT ON COLUMN are_on.place IS 'place in the podium at the end of the race';


--
-- Name: bets; Type: TABLE; Schema: public; Owner: bop2; Tablespace: 
--

CREATE TABLE bets (
    bet_id integer NOT NULL,
    user_id integer NOT NULL,
    horse_id integer NOT NULL,
    race_id integer NOT NULL,
    bet numeric NOT NULL,
    balance numeric,
    CONSTRAINT bet_bet_check CHECK ((bet > (0)::numeric))
);


ALTER TABLE bets OWNER TO bop2;

--
-- Name: bet_bet_it_seq; Type: SEQUENCE; Schema: public; Owner: bop2
--

CREATE SEQUENCE bet_bet_it_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE bet_bet_it_seq OWNER TO bop2;

--
-- Name: bet_bet_it_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: bop2
--

ALTER SEQUENCE bet_bet_it_seq OWNED BY bets.bet_id;


--
-- Name: horses; Type: TABLE; Schema: public; Owner: bop2; Tablespace: 
--

CREATE TABLE horses (
    horse_id integer NOT NULL,
    name character varying NOT NULL,
    speed numeric NOT NULL,
    CONSTRAINT horse_name_check CHECK (((char_length((name)::text) >= 1) AND (char_length((name)::text) <= 15))),
    CONSTRAINT horse_speed_check CHECK ((speed > (0)::numeric))
);


ALTER TABLE horses OWNER TO bop2;

--
-- Name: COLUMN horses.name; Type: COMMENT; Schema: public; Owner: bop2
--

COMMENT ON COLUMN horses.name IS 'pelo menos 1 caracter';


--
-- Name: horse_horse_id_seq; Type: SEQUENCE; Schema: public; Owner: bop2
--

CREATE SEQUENCE horse_horse_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE horse_horse_id_seq OWNER TO bop2;

--
-- Name: horse_horse_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: bop2
--

ALTER SEQUENCE horse_horse_id_seq OWNED BY horses.horse_id;


--
-- Name: horse_ranks; Type: TABLE; Schema: public; Owner: bop2; Tablespace: 
--

CREATE TABLE horse_ranks (
    horse_id integer,
    name character varying,
    rank bigint
);

ALTER TABLE ONLY horse_ranks REPLICA IDENTITY NOTHING;


ALTER TABLE horse_ranks OWNER TO bop2;

--
-- Name: races; Type: TABLE; Schema: public; Owner: bop2; Tablespace: 
--

CREATE TABLE races (
    race_id integer NOT NULL,
    laps integer DEFAULT 0 NOT NULL,
    time_created timestamp(0) without time zone DEFAULT now(),
    started boolean DEFAULT false NOT NULL,
    CONSTRAINT corrida_laps_check CHECK ((laps > 0)),
    CONSTRAINT race_started_not_null CHECK ((started IS NOT NULL))
);


ALTER TABLE races OWNER TO bop2;

--
-- Name: race_id_seq; Type: SEQUENCE; Schema: public; Owner: bop2
--

CREATE SEQUENCE race_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE race_id_seq OWNER TO bop2;

--
-- Name: race_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: bop2
--

ALTER SEQUENCE race_id_seq OWNED BY races.race_id;


--
-- Name: users; Type: TABLE; Schema: public; Owner: bop2; Tablespace: 
--

CREATE TABLE users (
    user_id bigint NOT NULL,
    name character varying NOT NULL,
    username character varying NOT NULL,
    pass character varying DEFAULT '123456'::character varying NOT NULL,
    admin boolean DEFAULT false NOT NULL,
    credits numeric DEFAULT 0 NOT NULL,
    CONSTRAINT original_is_admin CHECK (admin OR NOT (user_id = 1)),
    CONSTRAINT users_credits_check CHECK (credits >= (0)::numeric),
    CONSTRAINT users_name_check CHECK (char_length((name)::text) >= 1),
    CONSTRAINT users_pass_check CHECK (char_length((pass)::text) >= 6),
    CONSTRAINT users_pass_check1 CHECK (char_length((pass)::text) <= 255),
    CONSTRAINT users_username_check CHECK (char_length((username)::text) >= 3)
);


CREATE OR REPLACE FUNCTION changeBet (usid integer, hoid integer, raid integer, newbet numeric) 
RETURNS varchar AS $ret$
DECLARE
	previousBet	numeric;
	ret		varchar;
BEGIN
	previousBet := (SELECT bet FROM  bets WHERE user_id = usid AND horse_id = hoid AND race_id = raid LIMIT 1);

	IF newbet < 0 THEN
		RETURN 'Bets must be positive';
	END IF;

	IF (previousBet IS NULL) AND (newbet = 0) THEN
		--RETURN 'The inserted user didn''t have a bet on the inserted horse and race';
		RETURN 'OK';
	END IF;

	IF (SELECT balance FROM  bets WHERE user_id = usid AND horse_id = hoid AND race_id = raid LIMIT 1) IS NOT NULL THEN
		RETURN 'Race already finished and thus it''s not possible to change this bet';
	END IF;

	--The bet change/removal/creation is valid!

	IF newbet = 0 THEN --the bet exists and is to be deleted
		DELETE FROM bets
		WHERE user_id = usid AND race_id = raid AND horse_id = hoid;
	ELSE --the bet is to be created or changed
		IF previousBet IS NULL THEN --the bet didn't exist so it'll be created
			INSERT INTO bets VALUES (DEFAULT,usid,hoid,raid,newbet,NULL);
		ELSE --the bet exists so it'll be updated
			UPDATE bets
			SET bet = newbet
			WHERE user_id = usid AND horse_id = hoid AND race_id = raid;
		END IF;
	END IF;

	UPDATE users
	SET credits=credits-newbet+COALESCE(previousBet,0)
	WHERE user_id = usid;

	RETURN 'OK';
END;
$ret$ LANGUAGE plpgsql;



ALTER TABLE users OWNER TO bop2;

--
-- Name: COLUMN users.name; Type: COMMENT; Schema: public; Owner: bop2
--

COMMENT ON COLUMN users.name IS 'at least 1 character';


--
-- Name: COLUMN users.username; Type: COMMENT; Schema: public; Owner: bop2
--

COMMENT ON COLUMN users.username IS 'at least 3 characters';


--
-- Name: COLUMN users.pass; Type: COMMENT; Schema: public; Owner: bop2
--

COMMENT ON COLUMN users.pass IS 'at least 6 characters';


--
-- Name: users_user_id_seq; Type: SEQUENCE; Schema: public; Owner: bop2
--

CREATE SEQUENCE users_user_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE users_user_id_seq OWNER TO bop2;

--
-- Name: users_user_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: bop2
--

ALTER SEQUENCE users_user_id_seq OWNED BY users.user_id;


--
-- Name: bet_id; Type: DEFAULT; Schema: public; Owner: bop2
--

ALTER TABLE ONLY bets ALTER COLUMN bet_id SET DEFAULT nextval('bet_bet_it_seq'::regclass);


--
-- Name: horse_id; Type: DEFAULT; Schema: public; Owner: bop2
--

ALTER TABLE ONLY horses ALTER COLUMN horse_id SET DEFAULT nextval('horse_horse_id_seq'::regclass);


--
-- Name: race_id; Type: DEFAULT; Schema: public; Owner: bop2
--

ALTER TABLE ONLY races ALTER COLUMN race_id SET DEFAULT nextval('race_id_seq'::regclass);


--
-- Name: user_id; Type: DEFAULT; Schema: public; Owner: bop2
--

ALTER TABLE ONLY users ALTER COLUMN user_id SET DEFAULT nextval('users_user_id_seq'::regclass);


--
-- Data for Name: are_on; Type: TABLE DATA; Schema: public; Owner: bop2
--

INSERT INTO are_on VALUES (2, 6, 0.992892, 2);
INSERT INTO are_on VALUES (3, 6, 0.500061, 4);
INSERT INTO are_on VALUES (5, 6, 0.945139, 1);
INSERT INTO are_on VALUES (7, 6, 0.512868, 3);
INSERT INTO are_on VALUES (2, 3, 0.777566, 2);
INSERT INTO are_on VALUES (3, 3, 0.941424, 1);
INSERT INTO are_on VALUES (5, 3, 0.527845, 4);
INSERT INTO are_on VALUES (7, 3, 0.745738, 3);


--
-- Name: bet_bet_it_seq; Type: SEQUENCE SET; Schema: public; Owner: bop2
--

SELECT pg_catalog.setval('bet_bet_it_seq', 29, true);


--
-- Data for Name: bets; Type: TABLE DATA; Schema: public; Owner: bop2
--

INSERT INTO bets VALUES (20, 1, 2, 3, 50, NULL);
INSERT INTO bets VALUES (23, 1, 7, 3, 60, NULL);
INSERT INTO bets VALUES (28, 2, 5, 3, 45, NULL);


--
-- Name: horse_horse_id_seq; Type: SEQUENCE SET; Schema: public; Owner: bop2
--

SELECT pg_catalog.setval('horse_horse_id_seq', 7, true);


--
-- Data for Name: horses; Type: TABLE DATA; Schema: public; Owner: bop2
--

INSERT INTO horses VALUES (2, 'bolt', 10.1);
INSERT INTO horses VALUES (3, 'jolly jumper', 9);
INSERT INTO horses VALUES (5, 'Siemens', 11);
INSERT INTO horses VALUES (7, 'Ohm', 10.5);


--
-- Name: race_id_seq; Type: SEQUENCE SET; Schema: public; Owner: bop2
--

SELECT pg_catalog.setval('race_id_seq', 6, true);


--
-- Data for Name: races; Type: TABLE DATA; Schema: public; Owner: bop2
--

INSERT INTO races VALUES (6, 6, '2016-06-01 09:47:41', false);
INSERT INTO races VALUES (3, 4, '2016-05-29 15:28:32', false);


--
-- Data for Name: users; Type: TABLE DATA; Schema: public; Owner: bop2
--

INSERT INTO users VALUES (9, 'agwsadf', 'asdfg', '123456', false, 0);
INSERT INTO users VALUES (10, 'Johny', 'johny', '123456', false, 0);
INSERT INTO users VALUES (3, 'Alice Moreira', 'landinha', '123456', false, 1000000);
INSERT INTO users VALUES (2, 'Narciso Caldas', 'cisinho', '123456', false, 1000000);
INSERT INTO users VALUES (1, 'Luis Paulo', 'lpcsd', '123456', true, 1000000);


--
-- Name: users_user_id_seq; Type: SEQUENCE SET; Schema: public; Owner: bop2
--

SELECT pg_catalog.setval('users_user_id_seq', 10, true);


--
-- Name: bet_pkey; Type: CONSTRAINT; Schema: public; Owner: bop2; Tablespace: 
--

ALTER TABLE ONLY bets
    ADD CONSTRAINT bet_pkey PRIMARY KEY (bet_id);


--
-- Name: bet_user_id_horse_id_race_id_key; Type: CONSTRAINT; Schema: public; Owner: bop2; Tablespace: 
--

ALTER TABLE ONLY bets
    ADD CONSTRAINT bet_user_id_horse_id_race_id_key UNIQUE (user_id, horse_id, race_id);


--
-- Name: horse_name_key; Type: CONSTRAINT; Schema: public; Owner: bop2; Tablespace: 
--

ALTER TABLE ONLY horses
    ADD CONSTRAINT horse_name_key UNIQUE (name);


--
-- Name: horse_pkey; Type: CONSTRAINT; Schema: public; Owner: bop2; Tablespace: 
--

ALTER TABLE ONLY horses
    ADD CONSTRAINT horse_pkey PRIMARY KEY (horse_id);


--
-- Name: is_on_pkey; Type: CONSTRAINT; Schema: public; Owner: bop2; Tablespace: 
--

ALTER TABLE ONLY are_on
    ADD CONSTRAINT is_on_pkey PRIMARY KEY (horse_id, race_id);


--
-- Name: race_pkey; Type: CONSTRAINT; Schema: public; Owner: bop2; Tablespace: 
--

ALTER TABLE ONLY races
    ADD CONSTRAINT race_pkey PRIMARY KEY (race_id);


--
-- Name: users_pkey; Type: CONSTRAINT; Schema: public; Owner: bop2; Tablespace: 
--

ALTER TABLE ONLY users
    ADD CONSTRAINT users_pkey PRIMARY KEY (user_id);


--
-- Name: users_username_key; Type: CONSTRAINT; Schema: public; Owner: bop2; Tablespace: 
--

ALTER TABLE ONLY users
    ADD CONSTRAINT users_username_key UNIQUE (username);


--
-- Name: _RETURN; Type: RULE; Schema: public; Owner: bop2
--

CREATE RULE "_RETURN" AS
    ON SELECT TO horse_ranks DO INSTEAD  SELECT horses.horse_id,
    horses.name,
    COALESCE(sum((10 / foo.place)), (0)::bigint) AS rank
   FROM (horses
     LEFT JOIN ( SELECT are_on.horse_id,
            are_on.race_id,
            are_on.place
           FROM are_on
          WHERE (are_on.place IS NOT NULL)) foo USING (horse_id))
  GROUP BY horses.horse_id
  ORDER BY COALESCE(sum((10 / foo.place)), (0)::bigint) DESC, horses.horse_id;


--
-- Name: cant_delete_horses; Type: RULE; Schema: public; Owner: bop2
--

CREATE RULE cant_delete_horses AS
    ON DELETE TO horses DO INSTEAD  SELECT 'For the integrity of the program, horses cannot be deleted' AS "Error message";


--
-- Name: original's_immortality; Type: RULE; Schema: public; Owner: bop2
--

CREATE RULE "original's_immortality" AS
    ON DELETE TO users
   WHERE (old.user_id = 1) DO INSTEAD  SELECT 'Trying to kill The Originals is a task doomed to failure. Go home, you are drunk.' AS "Error: Futile action attempted";


--
-- Name: bet_horse_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: bop2
--

ALTER TABLE ONLY bets
    ADD CONSTRAINT bet_horse_id_fkey FOREIGN KEY (horse_id) REFERENCES horses(horse_id);


--
-- Name: bet_race_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: bop2
--

ALTER TABLE ONLY bets
    ADD CONSTRAINT bet_race_id_fkey FOREIGN KEY (race_id) REFERENCES races(race_id);


--
-- Name: bet_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: bop2
--

ALTER TABLE ONLY bets
    ADD CONSTRAINT bet_user_id_fkey FOREIGN KEY (user_id) REFERENCES users(user_id);


--
-- Name: is_on_horse_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: bop2
--

ALTER TABLE ONLY are_on
    ADD CONSTRAINT is_on_horse_id_fkey FOREIGN KEY (horse_id) REFERENCES horses(horse_id);


--
-- Name: is_on_race_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: bop2
--

ALTER TABLE ONLY are_on
    ADD CONSTRAINT is_on_race_id_fkey FOREIGN KEY (race_id) REFERENCES races(race_id);


--
-- Name: public; Type: ACL; Schema: -; Owner: bop2
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM bop2;
GRANT ALL ON SCHEMA public TO bop2;


--
-- PostgreSQL database dump complete
--

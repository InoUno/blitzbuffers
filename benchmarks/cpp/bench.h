#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

namespace Raw {
	enum class EntityType : uint8_t
	{
		Player,
		Enemy,
		Neutral
	};

	struct Position
	{
		float x;
		float y;
		float z;
	};

	struct Entity
	{
		EntityType type;
		Position position;
		std::string name;
		std::vector<Entity> related;
	};
};


const Raw::Entity BENCH_DATA = {
	Raw::EntityType::Player,
	{ 1.f, 2.f, 3.f },
	"Player",
	{
		{
			Raw::EntityType::Neutral,
			{ 4.f, 8.f, 16.f },
			"Rabbit",
		},
		{
			Raw::EntityType::Neutral,
			{ 4.f, 8.f, 16.f },
			"Rabbit",
		},
		{
			Raw::EntityType::Neutral,
			{ 4.f, 8.f, 16.f },
			"Rabbit",
		},
		{
			Raw::EntityType::Neutral,
			{ 4.f, 8.f, 16.f },
			"Rabbit",
		},
		{
			Raw::EntityType::Neutral,
			{ 4.f, 8.f, 16.f },
			"Rabbit",
		},
	}
};

const size_t CHECKSUM = 197;

struct Bench {
	virtual ~Bench() {}

	inline void use_value(size_t value) { sum += value; }

	virtual uint8_t* encode(size_t& len) = 0;
	virtual uint8_t* decode(uint8_t* buf, size_t len) = 0;

	virtual size_t use(uint8_t* decoded, size_t len) = 0;

	size_t sum = 0;
};


#include "bench.h"
#include <alpaca/alpaca.h>
#include <span>

namespace AlpacaContainer
{
	struct AlpacaBench : Bench
	{
		std::vector<uint8_t> bytes;
		std::error_code ec;

		AlpacaBench()
		{}

		virtual uint8_t* encode(size_t& len) override
		{
			bytes.clear();
			len = alpaca::serialize(BENCH_DATA, bytes);
			return bytes.data();
		}

		virtual uint8_t* decode(uint8_t* buf, size_t len) override
		{
			return buf;
		}

		virtual size_t use(uint8_t* decoded, size_t len) override {
			sum = 0;

			std::span<uint8_t> data(decoded, len);
			Raw::Entity entity = alpaca::deserialize<Raw::Entity>(data, ec);

			use_value((size_t)entity.type);
			use_value(entity.position.x);
			use_value(entity.position.y);
			use_value(entity.position.z);
			use_value(entity.name.length());

			auto related_entities = entity.related;
			use_value(related_entities.size());
			for (const auto& related : related_entities)
			{
				use_value((size_t)related.type);
				use_value(related.position.x);
				use_value(related.position.y);
				use_value(related.position.z);
				use_value(related.name.length());

			}

			return sum;
		}
	};


};
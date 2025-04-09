
#include "bench.h"

#include "blitzbuffers_generated.h"

namespace BlitzBuffersContainer
{
	using namespace blitzbuffers;
	using namespace bench_blitzbuffers;

	template<typename BufferBackend>
	struct BlitzBuffersBench : Bench
	{
		BufferBackend backend;

		BlitzBuffersBench(BufferBackend&& _backend) : backend(std::move(_backend))
		{}

		virtual uint8_t* encode(size_t& len) override
		{
			backend.clear();

			auto player = Entity::new_on(backend);
			player.set_type(EntityType::Player);
			player.position() = { BENCH_DATA.position.x, BENCH_DATA.position.y, BENCH_DATA.position.z };
			player.insert_name(BENCH_DATA.name);

			auto related_len = BENCH_DATA.related.size();
			auto related_entities = player.insert_related(related_len);
 
			for (size_t i = 0; i < related_len; i++)
			{
				auto& related_data = BENCH_DATA.related[i];
				auto related = related_entities[i];

				related.set_type(EntityType::Neutral);
				related.position() = { related_data.position.x, related_data.position.y, related_data.position.z };
				related.insert_name(related_data.name);
			}

			auto [length, buffer] = backend.build();
			len = length;
			return buffer;
		}

		virtual uint8_t* decode(uint8_t* buf, size_t len) override
		{
			if (!Entity::check(buf, len))
			{
				throw std::runtime_error("Invalid blitzbuffer");
			}
			return buf;
		}

		virtual size_t use(uint8_t* decoded, size_t len) override {
			sum = 0;

			auto entity = Entity::view_unchecked(decoded);

			use_value(to_underlying(entity.get_type()));
			use_value(entity.get_position().get_x());
			use_value(entity.get_position().get_y());
			use_value(entity.get_position().get_z());
			use_value(strlen(entity.get_name()));

			auto related_entities = entity.get_related();
			use_value(related_entities.length());
			for (const auto& related : related_entities)
			{
				use_value(to_underlying(related.get_type()));
				use_value(related.get_position().get_x());
				use_value(related.get_position().get_y());
				use_value(related.get_position().get_z());
				use_value(strlen(related.get_name()));
			}

			return sum;
		}
	};

};

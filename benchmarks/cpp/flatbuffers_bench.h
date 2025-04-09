
#include "bench.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers_generated.h"

namespace FlatBuffersContainer
{
	using namespace flatbuffers;
	using namespace bench_flatbuffers;

	struct FlatBuffersBench : Bench
	{
		FlatBufferBuilder fbb = FlatBufferBuilder(1024);

		FlatBuffersBench()
		{}

		virtual uint8_t* encode(size_t& len) override
		{
			fbb.Clear();

			int related_len = BENCH_DATA.related.size();
			std::vector<Offset<Entity>> offsets(related_len);

			for (size_t i = 0; i < related_len; i++)
			{
				auto& related = BENCH_DATA.related[i];
				Position pos(related.position.x, related.position.y, related.position.z);
				auto entity = CreateEntityDirect(fbb, EntityType_Neutral, &pos, related.name.c_str(), {});

				offsets[i] = entity;
			}

			Position pos(BENCH_DATA.position.x, BENCH_DATA.position.y, BENCH_DATA.position.z);
			auto entity = CreateEntityDirect(fbb, EntityType_Player, &pos, BENCH_DATA.name.c_str(), &offsets);

			fbb.Finish(entity);

			len = fbb.GetSize();

			return fbb.GetBufferPointer();
		}

		virtual uint8_t* decode(uint8_t* buf, size_t len) override
		{
			auto verifier = Verifier::Verifier(buf, len);
			if (!VerifyEntityBuffer(verifier))
			{
				throw std::runtime_error("Invalid flatbuffer");
			}
			return buf;
		}

		virtual size_t use(uint8_t* decoded, size_t len) override {
			sum = 0;

			auto entity = GetEntity(decoded);

			use_value((size_t)entity->type());
			use_value(entity->position()->x());
			use_value(entity->position()->y());
			use_value(entity->position()->z());
			use_value(entity->name()->size());

			auto related_entities = entity->related();
			use_value(related_entities->size());
			for (const auto& related : *related_entities)
			{
				use_value((size_t)related->type());
				use_value(related->position()->x());
				use_value(related->position()->y());
				use_value(related->position()->z());
				use_value(related->name()->size());

			}

			return sum;
		}
	};


};
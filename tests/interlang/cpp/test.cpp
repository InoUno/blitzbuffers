
#include <fstream>
#include <filesystem>
#include <vector>
#include <optional>
#include <format>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <memory>
#include <string>
#include <stdexcept>

// Taken from: https://stackoverflow.com/a/26221725/2236416
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

#include "./schema.h"

using namespace blitzbuffers;
using namespace tests_blitzbuffers;

template<typename T>
void write_entity(json json_entity, Entity::Builder<T> entity)
{
	std::string type_ = json_entity["type"];
	if (type_ == "Player")
	{
		entity.set_type(EntityType::Player);
	}
	else if (type_ == "Enemy")
	{
		entity.set_type(EntityType::Enemy);
	}
	else if (type_ == "Neutral")
	{
		entity.set_type(EntityType::Neutral);
	}

	entity.insert_name(json_entity["name"].template get<std::string>());
	entity.insert_aliases(json_entity["aliases"].template get<std::vector<std::string>>());
	entity.insert_numbers(json_entity["numbers"].template get<std::vector<std::vector<std::vector<uint16_t>>>>());

	auto position = json_entity["position"];
	if (position.is_string())
	{
		if (position == "NoPosition")
		{
			entity.position().make_NoPosition();
		}
	}
	else {
		if (const auto posIter = position.find("InlinePosition"); posIter != position.end())
		{
			auto pos = *posIter;
			auto out_pos = entity.position().make_InlinePosition();
			out_pos.set_x(pos["x"]);
			out_pos.set_y(pos["y"]);
			out_pos.set_z(pos["z"]);
		}
		else if (const auto posIter = position.find("Position"); posIter != position.end())
		{
			auto pos = *posIter;
			auto out_pos = entity.position().make_Position()._0();
			out_pos.set_x(pos["x"]);
			out_pos.set_y(pos["y"]);
			out_pos.set_z(pos["z"]);
		}
		else if (const auto posIter = position.find("TuplePosition"); posIter != position.end())
		{
			auto pos = *posIter;
			auto out_pos = entity.position().make_TuplePosition();
			out_pos.set_0(pos[0]);
			out_pos.set_1(pos[1]);
			out_pos.set_2(pos[2]);
		}
	}

	auto related_vec = json_entity["related"].template get<std::vector<json>>();
	if (related_vec.empty())
	{
		return;
	}

	auto related_out = entity.insert_related(related_vec.size());
	for (auto i = 0; i < related_vec.size(); i++)
	{
		write_entity(related_vec[i], related_out[i]);
	}
}

void generate(const std::filesystem::path& data_path)
{
	auto in_path = data_path / "data.json";

	std::ifstream f(in_path);
	auto data = json::parse(f);

	auto backend = ExpandableBufferBackend();
	auto entity = Entity::new_on(backend);

	write_entity(data, entity);

	auto [length, buffer] = backend.build();

	auto out_path = data_path / "cpp.data";
	std::ofstream fout;
	fout.open(out_path, std::ios::binary | std::ios::out);
	fout.write((char*)buffer, length);
	fout.flush();
	fout.close();

	std::cout << "Generated file: " << out_path << std::endl;
}

std::optional<std::vector<std::string>> compare_entities(Entity::Viewer original, Entity::Viewer encoded)
{
	if (original.get_type() != encoded.get_type())
	{
		std::stringstream ss;
		ss << "Types are not the same. Original: '" << original.get_type() << "', Encoded: '" << encoded.get_type() << "'";
		return { { ss.str() } };
	}

	if (strcmp(original.get_name(), encoded.get_name()) != 0)
	{
		std::stringstream ss;
		ss << "Names are not the same. Original: '" << original.get_name() << "', Encoded: '" << encoded.get_name() << "'";
		return { { ss.str() } };
	}

	if (original.get_position() != encoded.get_position())
	{
		std::stringstream ss;
		ss << "Positions are not the same. Original: '" << original.get_position() << "', Encoded: '" << encoded.get_position() << "'";
		return { { ss.str() } };
	}

	if (original.get_aliases() != encoded.get_aliases())
	{
		std::stringstream ss;
		ss << "Aliases are not the same. Original: '" << original.get_aliases() << "', Encoded: '" << encoded.get_aliases() << "'";
		return { { ss.str() } };
	}

	if (original.get_numbers() != encoded.get_numbers())
	{
		std::stringstream ss;
		ss << "Numbers are not the same. Original: '" << original.get_numbers() << "', Encoded: '" << encoded.get_numbers() << "'";
		return { { ss.str() } };
	}

	auto org_related = original.get_related();
	auto enc_related = encoded.get_related();
	if (org_related.length() != enc_related.length())
	{
		std::stringstream ss;
		ss << "Related lengths are not the same. Original: : '" << org_related.length() << "', Encoded: '" << enc_related.length() << "'";
		return { { ss.str() } };
	}

	for (auto i = 0; i < org_related.length(); i++)
	{
		auto comp = compare_entities(org_related[i], enc_related[i]);
		if (comp.has_value())
		{
			auto errs = comp.value();
			errs.push_back(string_format("Related entity %u are different.", i));
			return { errs };
		}
	}

	return std::nullopt;
}

bool check(const std::filesystem::path& data_path)
{
	auto in_path = data_path / "data.json";

	std::ifstream f(in_path);
	auto data = json::parse(f);

	auto backend = ExpandableBufferBackend();
	auto entity = Entity::new_on(backend);

	write_entity(data, entity);

	auto [length, buffer] = backend.build();

	auto org_view = Entity::view(buffer, length).value();

	for (const auto& entry : std::filesystem::directory_iterator(data_path))
	{
		if (entry.path().extension() != ".data")
		{
			continue;
		}

		std::ifstream ifs(entry.path(), std::ios::binary | std::ios::ate);
		std::ifstream::pos_type length = ifs.tellg();

		if (length == 0) {
			continue;
		}

		std::vector<uint8_t> buffer(length);

		ifs.seekg(0, std::ios::beg);
		ifs.read((char*)buffer.data(), length);

		auto enc_view = Entity::view(buffer.data(), buffer.size());
		if (!enc_view.has_value())
		{
			std::cerr << "Failed check: " << entry.path() << std::endl;
			return false;
		}

		auto compOpt = compare_entities(org_view, enc_view.value());
		if (compOpt.has_value())
		{
			auto& comp = compOpt.value();
			for (auto iter = comp.rbegin(); iter != comp.rend(); iter++)
			{
				std::cerr << *iter << std::endl;
			}
			return false;
		}

		std::cout << "Checked file: " << entry.path() << std::endl;
	}

	return true;
}

std::filesystem::path get_data_path()
{
	std::filesystem::path data_path = __FILE__;
	data_path = data_path.parent_path().parent_path().append("data/general");
	return data_path;
}

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		generate(get_data_path());
		return 0;
	}

	if (strcmp(argv[1], "c") == 0)
	{
		if (argc <= 2) {
			return !check(get_data_path());
		}
		for (auto i = 2; i < argc; i++)
		{
			if (!check(argv[i]))
			{
				return 1;
			}
		}
		return 0;
	}
	else if (strcmp(argv[1], "g") == 0)
	{
		if (argc <= 2) {
			generate(get_data_path());
			return 0;
		}
		for (auto i = 2; i < argc; i++)
		{
			generate(argv[i]);
		}
		return 0;
	}
	else {
		std::cerr << "Unexpected argument: " << argv[2] << std::endl;
		return 1;
	}
}

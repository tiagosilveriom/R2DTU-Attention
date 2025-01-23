#pragma once

#include <memory>
#include <vector>

#include "del/types.hpp"


extern "C" {
	typedef struct GVC_s GVC_t;
}

namespace del
{
	class visualizer
	{
	public:
		static visualizer & get_instance();

		visualizer(visualizer const &) = delete;
		visualizer & operator=(visualizer const &) = delete;
		visualizer(visualizer &&) = delete;
		visualizer & operator=(visualizer &&) = delete;

		struct rendered_data_deleter
		{
			void operator ()(char * data);
		};

		std::unique_ptr<char, rendered_data_deleter> visualize(state_id s);
		std::unique_ptr<char, rendered_data_deleter> visualize(action_id a);

	private:
		visualizer();
		~visualizer() = default;

		struct context_deleter
		{
			void operator ()(GVC_t * gvc);
		};

		std::unique_ptr<GVC_t, context_deleter> gvc;
	};
}

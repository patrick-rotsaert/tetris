#pragma once

#include <memory>

class TuiApp final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	TuiApp();
	~TuiApp() noexcept;

	int run();
};

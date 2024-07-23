#include <QtTest>

#include "test_main.hpp"

int main(int argc, char** argv)
{
	unsigned int status = 0;
	auto ASSERT_TEST    = [&status, argc, argv](std::unique_ptr<QObject> obj) {
        status
            |= static_cast<unsigned int>(QTest::qExec(obj.get(), argc, argv));
	};

	test_main(ASSERT_TEST);

	return status;
}

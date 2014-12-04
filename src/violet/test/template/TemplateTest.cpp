#include "violet/test/template/TemplateTest.h"

#include "violet/core/template/TemplateUtilities.h"
#include "violet/core/template/TupleUtilities.h"
#include "violet/test/core/TestEvaluator.h"
#include "violet/test/core/TestFactory.h"

#include <tuple>
#include <vector>
#include <string>

using namespace Violet;

struct Int
{
	Int(int i) : m_i(i) {}
	operator int() { return m_i; }
	friend std::ostream & operator<<(std::ostream & stream, const Int & i) { return stream << i.m_i; }
	friend bool operator==(const Int & lhs, const Int & rhs) { return true; }
	int m_i;
};

struct Zero : Int { Zero() : Int(0) {} };
struct One : Int { One() : Int(1) {} };

void TemplateTests::run(TestEvaluator & evaluator)
{
	TestFactory::makeStatelessSuite("template tests", std::forward_as_tuple(
		TestFactory::makeStateless("true_type", true, []() { return true_type::value; }),
		TestFactory::makeStateless("false_type", false, []() { return false_type::value; }),
		TestFactory::makeStateless("unsigned_constant", static_cast<unsigned>(42), []() { return unsigned_constant<42>::value; }),
		TestFactory::makeStateless("if_else true", Zero(), []() { return if_else<true, Zero, One>::type(); }),
		TestFactory::makeStateless("if_else false", One(), []() { return if_else<false, Zero, One>::type(); }),
		TestFactory::makeStateless("unsigned_if_else true", static_cast<unsigned>(0), []() { return unsigned_if_else<true, 0, 1>::value; }),
		TestFactory::makeStateless("unsigned_if_else false", static_cast<unsigned>(1), []() { return unsigned_if_else<false, 0, 1>::value; }),
		TestFactory::makeStateless("equal false", false, []() { return equal<int, float>::value; }),
		TestFactory::makeStateless("equal true", true, []() { return equal<int, int>::value; }),
		TestFactory::makeStateless("has_type false", false, []() { return has_type<std::tuple<int, float>, char>::value; }),
		TestFactory::makeStateless("has_type true", true, []() { return has_type<std::tuple<int, float, char>, float>::value; }),
		TestFactory::makeStateless("is_unique false", false, []() { return is_unique<std::tuple<int, float, int>>::value; }),
		TestFactory::makeStateless("is_unique true", true, []() { return is_unique<std::tuple<int, float, char>>::value; }),
		TestFactory::makeStateless("index_of head", static_cast<unsigned>(0), []() { return index_of<int, std::tuple<int, float>>::value; }),
		TestFactory::makeStateless("index_of tail", static_cast<unsigned>(1), []() { return index_of<float, std::tuple<int, float>>::value; }),
		TestFactory::makeStateless("get", 42, []() { return get<int>(std::make_tuple(42, 5.f)); }),
		TestFactory::makeStateless("get", 42.f, []() { return get<float>(std::make_tuple(5, 42.f)); }),
		TestFactory::makeStateless("extract", std::tuple<int, char>(5, 'x'), []() {
			auto t1 = std::make_tuple(5, 42.f, 'x');
			std::tuple<int, char> t2;
			extract(t1, t2);
			return t2;
		})
	)).evaluate(evaluator);
}




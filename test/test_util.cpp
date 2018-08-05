#include "gtest/gtest.h"

#include <string>

#include <boost/lexical_cast.hpp>

#include "util/util.h"
#include "util/type_conversion.h"

// Strings
// util_strings.h

TEST(Strings, TrimLeft) {
    std::string str{"\t\n\r Test \t\n\r"};
    util::trim_left(str);

    EXPECT_STRCASEEQ(str.c_str(), "Test \t\n\r");
}
TEST(Strings, TrimLeftIter) {
    std::string str{"\t\n\r Test \t\n\r"};

    auto begin = str.begin();
    auto end = str.end();
    util::trim_left(begin, end);
    std::string str2{begin, end};

    EXPECT_STRCASEEQ(str2.c_str(), "Test \t\n\r");
}

TEST(Strings, TrimRight) {
    std::string str{"\t\n\r Test \t\n\r"};
    util::trim_right(str);

    EXPECT_STRCASEEQ(str.c_str(), "\t\n\r Test");
}
TEST(Strings, TrimRightIter) {
    std::string str{"\t\n\r Test \t\n\r"};

    auto begin = str.begin();
    auto end = str.end();
    util::trim_right(begin, end);
    std::string str2{begin, end};

    EXPECT_STRCASEEQ(str2.c_str(), "\t\n\r Test");
}

TEST(Strings, TrimBoth) {
    std::string str{"\t\n\r Test \t\n\r"};
    util::trim_both(str);

    EXPECT_STRCASEEQ(str.c_str(), "Test");
}
TEST(Strings, TrimBothIter) {
    std::string str{"\t\n\r Test \t\n\r"};

    auto begin = str.begin();
    auto end = str.end();
    util::trim_both(begin, end);
    std::string str2{begin, end};

    EXPECT_STRCASEEQ(str2.c_str(), "Test");
}

TEST(Strings, TrimAll) {
    std::string str{"\t\n\r Test \t\n\r"};
    // Trim nothing
    util::trim_all(str, [](unsigned char c) { return false; });
    EXPECT_STRCASEEQ(str.c_str(), "\t\n\r Test \t\n\r");

    str = "\t\n\r Test \t\n\r";
    // Trim whitespace
    util::trim_all(str, [](unsigned char c) { return c == ' '; });
    EXPECT_STRCASEEQ(str.c_str(), "\t\n\rTest\t\n\r");

    str = "\t\n\r Test \t\n\r";
    // Trim all space characters
    util::trim_all(str, [](unsigned char c) { return ::isspace(c); });
    EXPECT_STRCASEEQ(str.c_str(), "Test");

    str = "\t\n\r Test \t\n\r";
    // Trim all characters
    util::trim_all(str, [](unsigned char c) { return true; });
    EXPECT_TRUE(str.empty());
}

TEST(Strings, Split) {
    {
        // Case 1: delimiter not part of the string, empty strings not allowed, no empty strings encountered
        std::vector<std::string> out;
        bool encountered_empty = util::split("This is a test.", 'x', std::back_inserter(out), false);

        EXPECT_FALSE(encountered_empty);
        ASSERT_FALSE(out.empty());
        EXPECT_EQ(out.size(), 1);
        EXPECT_STREQ(out.at(0).c_str(), "This is a test.");
    }
    {
        // Case 2: delimiter part of the string, empty strings not allowed, no empty strings encountered
        std::vector<std::string> out;
        bool encountered_empty = util::split("This is a test.", ' ', std::back_inserter(out), false);

        EXPECT_FALSE(encountered_empty);
        ASSERT_FALSE(out.empty());
        EXPECT_EQ(out.size(), 4);
        EXPECT_STREQ(out.at(0).c_str(), "This");
        EXPECT_STREQ(out.at(1).c_str(), "is");
        EXPECT_STREQ(out.at(2).c_str(), "a");
        EXPECT_STREQ(out.at(3).c_str(), "test.");
    }
    {
        // Case 3: delimiter part of the string, empty strings not allowed, empty string encountered
        std::vector<std::string> out;
        bool encountered_empty = util::split("This is  a test.", ' ', std::back_inserter(out), false);

        EXPECT_TRUE(encountered_empty);
        ASSERT_FALSE(out.empty());
        EXPECT_EQ(out.size(), 4);
        EXPECT_STREQ(out.at(0).c_str(), "This");
        EXPECT_STREQ(out.at(1).c_str(), "is");
        EXPECT_STREQ(out.at(2).c_str(), "a");
        EXPECT_STREQ(out.at(3).c_str(), "test.");
    }
    {
        // Case 4: delimiter part of the string, empty strings allowed, empty string encountered
        std::vector<std::string> out;
        bool encountered_empty = util::split("This is  a test.", ' ', std::back_inserter(out), true);

        EXPECT_TRUE(encountered_empty);
        ASSERT_FALSE(out.empty());
        EXPECT_EQ(out.size(), 5);
        EXPECT_STREQ(out.at(0).c_str(), "This");
        EXPECT_STREQ(out.at(1).c_str(), "is");
        EXPECT_STREQ(out.at(2).c_str(), "");
        EXPECT_STREQ(out.at(3).c_str(), "a");
        EXPECT_STREQ(out.at(4).c_str(), "test.");
    }
}

TEST(Strings, SplitVector) {
    // Tests the overload of split that returns an optional vector
    {
        // Case 1: delimiter not part of the string, empty strings not allowed, no empty strings encountered
        auto out = util::split("This is a test.", 'x', false);

        ASSERT_NE(out, std::nullopt);
        ASSERT_FALSE(out->empty());
        EXPECT_EQ(out->size(), 1);
        EXPECT_STREQ(out->at(0).c_str(), "This is a test.");
    }
    {
        // Case 2: delimiter part of the string, empty strings not allowed, no empty strings encountered
        auto out = util::split("This is a test.", ' ', false);

        ASSERT_NE(out, std::nullopt);
        ASSERT_FALSE(out->empty());
        EXPECT_EQ(out->size(), 4);
        EXPECT_STREQ(out->at(0).c_str(), "This");
        EXPECT_STREQ(out->at(1).c_str(), "is");
        EXPECT_STREQ(out->at(2).c_str(), "a");
        EXPECT_STREQ(out->at(3).c_str(), "test.");
    }
    {
        // Case 3: delimiter part of the string, empty strings not allowed, empty string encountered
        auto out = util::split("This is  a test.", ' ', false);

        EXPECT_EQ(out, std::nullopt);
    }
    {
        // Case 4: delimiter part of the string, empty strings allowed, empty string encountered
        auto out = util::split("This is  a test.", ' ', true);

        ASSERT_NE(out, std::nullopt);
        ASSERT_FALSE(out->empty());
        EXPECT_EQ(out->size(), 5);
        EXPECT_STREQ(out->at(0).c_str(), "This");
        EXPECT_STREQ(out->at(1).c_str(), "is");
        EXPECT_STREQ(out->at(2).c_str(), "");
        EXPECT_STREQ(out->at(3).c_str(), "a");
        EXPECT_STREQ(out->at(4).c_str(), "test.");
    }
}

// Type conversion

class SerializableClass {
public:
    std::string to_string() const { return "SerializableClass"; }
};

// to_string.h

TEST(TypeConversion, ToString) {
    EXPECT_STREQ(util::to_string("Test")->c_str(), "Test");
    EXPECT_STREQ(util::to_string(5)->c_str(), "5");
    EXPECT_STREQ(util::to_string(-5)->c_str(), "-5");

    SerializableClass s;
    EXPECT_STREQ(util::to_string(s)->c_str(), "SerializableClass");
}

// type_conversion.h

TEST(TypeConversion, ToType) {
    // nullopt returned for words, empty strings and over- & underflow
    EXPECT_EQ(util::to_type<int>("Test"), std::nullopt);
    EXPECT_EQ(util::to_type<int>(""), std::nullopt);
    EXPECT_EQ(util::to_type<int>("10000000000000000000000000"), std::nullopt);
    EXPECT_EQ(util::to_type<int>("-10000000000000000000000000"), std::nullopt);

    // convert string to integers
    EXPECT_EQ(*util::to_type<int>("1"), 1);
    EXPECT_EQ(*util::to_type<int>("-1"), -1);
    EXPECT_EQ(*util::to_type<int>("016"), 14);
    EXPECT_EQ(*util::to_type<int>("-016"), -14);
    EXPECT_EQ(*util::to_type<int>("0x16"), 22);
    EXPECT_EQ(*util::to_type<int>("-0x16"), -22);
    EXPECT_EQ(*util::to_type<int>("1.2345"), 1);

    // convert string to floating point types
    EXPECT_DOUBLE_EQ(*util::to_type<float>("1.2345"), 1.2345f);
    EXPECT_DOUBLE_EQ(*util::to_type<float>("-1.2345"), -1.2345f);
    EXPECT_DOUBLE_EQ(*util::to_type<double>("1.2345"), 1.2345);
    EXPECT_DOUBLE_EQ(*util::to_type<double>("-1.2345"), -1.2345);

    // convert integers to strings
    EXPECT_STREQ(util::to_type<std::string>(1)->c_str(), "1");
    EXPECT_STREQ(util::to_type<std::string>(-1)->c_str(), "-1");

    // since float to string may add zeroes to the output we test this by converting back using lexial_cast
    {
        float f = boost::lexical_cast<float>(*util::to_type<std::string>(1.2345f));
        EXPECT_DOUBLE_EQ(f, 1.2345f);
    }
    {
        float f = boost::lexical_cast<float>(*util::to_type<std::string>(-1.2345f));
        EXPECT_DOUBLE_EQ(f, -1.2345f);
    }
}

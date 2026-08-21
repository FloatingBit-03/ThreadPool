#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#include "config/config_value.hpp"


namespace packetforge::config
{


// ==========================================================
// Construction
// ==========================================================

TEST(ConfigValueTest, DefaultConstructionCreatesStringValue)
{
    ConfigValue value;

    EXPECT_TRUE(value.isString());
    EXPECT_FALSE(value.isInteger());
    EXPECT_FALSE(value.isBoolean());

    EXPECT_EQ(value.asString(), "");
}


TEST(ConfigValueTest, ConstructsStringValue)
{
    ConfigValue value(
        std::string{"127.0.0.1"}
    );

    EXPECT_TRUE(value.isString());
    EXPECT_FALSE(value.isInteger());
    EXPECT_FALSE(value.isBoolean());

    EXPECT_EQ(
        value.asString(),
        "127.0.0.1"
    );
}


TEST(ConfigValueTest, ConstructsCStringValue)
{
    ConfigValue value(
        "localhost"
    );

    EXPECT_TRUE(value.isString());
    EXPECT_EQ(
        value.asString(),
        "localhost"
    );
}


TEST(ConfigValueTest, ConstructsIntegerValue)
{
    ConfigValue value(
        std::int64_t{9000}
    );

    EXPECT_FALSE(value.isString());
    EXPECT_TRUE(value.isInteger());
    EXPECT_FALSE(value.isBoolean());

    EXPECT_EQ(
        value.asInteger(),
        9000
    );
}


TEST(ConfigValueTest, ConstructsBooleanValue)
{
    ConfigValue value(
        true
    );

    EXPECT_FALSE(value.isString());
    EXPECT_FALSE(value.isInteger());
    EXPECT_TRUE(value.isBoolean());

    EXPECT_TRUE(
        value.asBoolean()
    );
}


// ==========================================================
// Type Inspection
// ==========================================================

TEST(ConfigValueTest, StringValueReportsOnlyStringType)
{
    ConfigValue value(
        "packetforge"
    );

    EXPECT_TRUE(value.isString());
    EXPECT_FALSE(value.isInteger());
    EXPECT_FALSE(value.isBoolean());
}


TEST(ConfigValueTest, IntegerValueReportsOnlyIntegerType)
{
    ConfigValue value(
        std::int64_t{42}
    );

    EXPECT_FALSE(value.isString());
    EXPECT_TRUE(value.isInteger());
    EXPECT_FALSE(value.isBoolean());
}


TEST(ConfigValueTest, BooleanValueReportsOnlyBooleanType)
{
    ConfigValue value(
        false
    );

    EXPECT_FALSE(value.isString());
    EXPECT_FALSE(value.isInteger());
    EXPECT_TRUE(value.isBoolean());
}


// ==========================================================
// Value Access
// ==========================================================

TEST(ConfigValueTest, ReturnsStoredString)
{
    const std::string expected =
        "127.0.0.1";

    ConfigValue value(
        expected
    );

    EXPECT_EQ(
        value.asString(),
        expected
    );
}


TEST(ConfigValueTest, ReturnsStoredInteger)
{
    const std::int64_t expected =
        65535;

    ConfigValue value(
        expected
    );

    EXPECT_EQ(
        value.asInteger(),
        expected
    );
}


TEST(ConfigValueTest, ReturnsStoredBoolean)
{
    ConfigValue trueValue(true);
    ConfigValue falseValue(false);

    EXPECT_TRUE(
        trueValue.asBoolean()
    );

    EXPECT_FALSE(
        falseValue.asBoolean()
    );
}


// ==========================================================
// Wrong-Type Access
// ==========================================================

TEST(ConfigValueTest, StringAccessOnIntegerThrows)
{
    ConfigValue value(
        std::int64_t{9000}
    );

    EXPECT_THROW(
        value.asString(),
        std::runtime_error
    );
}


TEST(ConfigValueTest, IntegerAccessOnStringThrows)
{
    ConfigValue value(
        "9000"
    );

    EXPECT_THROW(
        value.asInteger(),
        std::runtime_error
    );
}


TEST(ConfigValueTest, BooleanAccessOnStringThrows)
{
    ConfigValue value(
        "true"
    );

    EXPECT_THROW(
        value.asBoolean(),
        std::runtime_error
    );
}


// ==========================================================
// Copy Semantics
// ==========================================================

TEST(ConfigValueTest, CopyConstructorPreservesValue)
{
    ConfigValue original(
        std::int64_t{8080}
    );

    ConfigValue copy(
        original
    );

    EXPECT_TRUE(copy.isInteger());

    EXPECT_EQ(
        copy.asInteger(),
        8080
    );

    EXPECT_EQ(
        original.asInteger(),
        8080
    );
}


TEST(ConfigValueTest, CopyAssignmentPreservesValue)
{
    ConfigValue original(
        "127.0.0.1"
    );

    ConfigValue copy;

    copy = original;

    EXPECT_TRUE(copy.isString());

    EXPECT_EQ(
        copy.asString(),
        "127.0.0.1"
    );
}


// ==========================================================
// Move Semantics
// ==========================================================

TEST(ConfigValueTest, MoveConstructorTransfersValue)
{
    ConfigValue original(
        std::int64_t{9000}
    );

    ConfigValue moved(
        std::move(original)
    );

    EXPECT_TRUE(
        moved.isInteger()
    );

    EXPECT_EQ(
        moved.asInteger(),
        9000
    );
}


TEST(ConfigValueTest, MoveAssignmentTransfersValue)
{
    ConfigValue original(
        true
    );

    ConfigValue moved;

    moved =
        std::move(original);

    EXPECT_TRUE(
        moved.isBoolean()
    );

    EXPECT_TRUE(
        moved.asBoolean()
    );
}


} // namespace packetforge::config
#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "config/config.hpp"


namespace packetforge::config
{


class ConfigTest : public ::testing::Test
{
protected:

    Config config;
};


// ----------------------------------------------------------
// Construction
// ----------------------------------------------------------

TEST_F(
    ConfigTest,
    DefaultConstructionCreatesEmptyConfig
)
{
    EXPECT_FALSE(
        config.contains("server.port")
    );

    EXPECT_EQ(
        config.get("server.port"),
        nullptr
    );
}


// ----------------------------------------------------------
// Contains
// ----------------------------------------------------------

TEST_F(
    ConfigTest,
    ContainsReturnsFalseForMissingKey
)
{
    EXPECT_FALSE(
        config.contains("missing")
    );
}


TEST_F(
    ConfigTest,
    ContainsReturnsTrueForExistingKey
)
{
    config.set(
        "server.port",
        ConfigValue(static_cast<std::int64_t>(8080))
    );

    EXPECT_TRUE(
        config.contains("server.port")
    );
}


// ----------------------------------------------------------
// Set
// ----------------------------------------------------------

TEST_F(
    ConfigTest,
    SetStoresStringValue
)
{
    config.set(
        "server.host",
        ConfigValue("127.0.0.1")
    );

    EXPECT_TRUE(
        config.contains("server.host")
    );
}


TEST_F(
    ConfigTest,
    SetStoresIntegerValue
)
{
    config.set(
        "server.port",
        ConfigValue(static_cast<std::int64_t>(8080))
    );

    EXPECT_TRUE(
        config.contains("server.port")
    );
}


TEST_F(
    ConfigTest,
    SetStoresBooleanValue
)
{
    config.set(
        "server.enabled",
        ConfigValue(true)
    );

    EXPECT_TRUE(
        config.contains("server.enabled")
    );
}


// ----------------------------------------------------------
// Get
// ----------------------------------------------------------

TEST_F(
    ConfigTest,
    GetReturnsStoredStringValue
)
{
    config.set(
        "server.host",
        ConfigValue("127.0.0.1")
    );

    const ConfigValue* value =
        config.get("server.host");

    ASSERT_NE(
        value,
        nullptr
    );

    EXPECT_TRUE(
        value->isString()
    );

    EXPECT_EQ(
        value->asString(),
        "127.0.0.1"
    );
}


TEST_F(
    ConfigTest,
    GetReturnsStoredIntegerValue
)
{
    config.set(
        "server.port",
        ConfigValue(static_cast<std::int64_t>(8080))
    );

    const ConfigValue* value =
        config.get("server.port");

    ASSERT_NE(
        value,
        nullptr
    );

    EXPECT_TRUE(
        value->isInteger()
    );

    EXPECT_EQ(
        value->asInteger(),
        8080
    );
}


TEST_F(
    ConfigTest,
    GetReturnsStoredBooleanValue
)
{
    config.set(
        "server.enabled",
        ConfigValue(true)
    );

    const ConfigValue* value =
        config.get("server.enabled");

    ASSERT_NE(
        value,
        nullptr
    );

    EXPECT_TRUE(
        value->isBoolean()
    );

    EXPECT_TRUE(
        value->asBoolean()
    );
}


// ----------------------------------------------------------
// Missing Key
// ----------------------------------------------------------

TEST_F(
    ConfigTest,
    GetReturnsNullptrForMissingKey
)
{
    const ConfigValue* value =
        config.get("missing");

    EXPECT_EQ(
        value,
        nullptr
    );
}


// ----------------------------------------------------------
// Update
// ----------------------------------------------------------

TEST_F(
    ConfigTest,
    SetReplacesExistingValue
)
{
    config.set(
        "server.port",
        ConfigValue(static_cast<std::int64_t>(8080))
    );

    config.set(
        "server.port",
        ConfigValue(static_cast<std::int64_t>(9090))
    );

    const ConfigValue* value =
        config.get("server.port");

    ASSERT_NE(
        value,
        nullptr
    );

    EXPECT_TRUE(
        value->isInteger()
    );

    EXPECT_EQ(
        value->asInteger(),
        9090
    );
}


TEST_F(
    ConfigTest,
    SetCanReplaceValueWithDifferentType
)
{
    config.set(
        "server.value",
        ConfigValue(static_cast<std::int64_t>(100))
    );

    config.set(
        "server.value",
        ConfigValue("updated")
    );

    const ConfigValue* value =
        config.get("server.value");

    ASSERT_NE(
        value,
        nullptr
    );

    EXPECT_TRUE(
        value->isString()
    );

    EXPECT_EQ(
        value->asString(),
        "updated"
    );
}


// ----------------------------------------------------------
// Multiple Values
// ----------------------------------------------------------

TEST_F(
    ConfigTest,
    StoresMultipleValuesIndependently
)
{
    config.set(
        "server.host",
        ConfigValue("127.0.0.1")
    );

    config.set(
        "server.port",
        ConfigValue(static_cast<std::int64_t>(8080))
    );

    config.set(
        "server.enabled",
        ConfigValue(true)
    );

    const ConfigValue* host =
        config.get("server.host");

    const ConfigValue* port =
        config.get("server.port");

    const ConfigValue* enabled =
        config.get("server.enabled");

    ASSERT_NE(host, nullptr);
    ASSERT_NE(port, nullptr);
    ASSERT_NE(enabled, nullptr);

    EXPECT_EQ(
        host->asString(),
        "127.0.0.1"
    );

    EXPECT_EQ(
        port->asInteger(),
        8080
    );

    EXPECT_TRUE(
        enabled->asBoolean()
    );
}


// ----------------------------------------------------------
// Clear
// ----------------------------------------------------------

TEST_F(
    ConfigTest,
    ClearRemovesAllValues
)
{
    config.set(
        "server.host",
        ConfigValue("127.0.0.1")
    );

    config.set(
        "server.port",
        ConfigValue(static_cast<std::int64_t>(8080))
    );

    config.set(
        "server.enabled",
        ConfigValue(true)
    );

    config.clear();

    EXPECT_FALSE(
        config.contains("server.host")
    );

    EXPECT_FALSE(
        config.contains("server.port")
    );

    EXPECT_FALSE(
        config.contains("server.enabled")
    );

    EXPECT_EQ(
        config.get("server.host"),
        nullptr
    );

    EXPECT_EQ(
        config.get("server.port"),
        nullptr
    );

    EXPECT_EQ(
        config.get("server.enabled"),
        nullptr
    );
}


TEST_F(
    ConfigTest,
    ClearOnEmptyConfigIsSafe
)
{
    EXPECT_NO_THROW(
        config.clear()
    );

    EXPECT_EQ(
        config.get("missing"),
        nullptr
    );
}


// ----------------------------------------------------------
// Key Independence
// ----------------------------------------------------------

TEST_F(
    ConfigTest,
    DifferentKeysRemainIndependent
)
{
    config.set(
        "server.port",
        ConfigValue(static_cast<std::int64_t>(8080))
    );

    config.set(
        "client.port",
        ConfigValue(static_cast<std::int64_t>(9090))
    );

    const ConfigValue* serverPort =
        config.get("server.port");

    const ConfigValue* clientPort =
        config.get("client.port");

    ASSERT_NE(serverPort, nullptr);
    ASSERT_NE(clientPort, nullptr);

    EXPECT_EQ(
        serverPort->asInteger(),
        8080
    );

    EXPECT_EQ(
        clientPort->asInteger(),
        9090
    );
}


} // namespace packetforge::config
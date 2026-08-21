#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>

#include "config/config_loader.hpp"

namespace packetforge::config
{

class ConfigLoaderTest : public ::testing::Test
{
protected:

    const std::string testFile =
        "build/test_config_loader.conf";

    void TearDown() override
    {
        std::remove(testFile.c_str());
    }

    void writeConfig(
        const std::string& contents
    )
    {
        std::ofstream file(testFile);

        ASSERT_TRUE(file.is_open());

        file << contents;

        file.close();
    }
};


// ----------------------------------------------------------
// Construction
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    DefaultConstructionSucceeds
)
{
    ConfigLoader loader;

    SUCCEED();
}


// ----------------------------------------------------------
// Empty Configuration
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    LoadsEmptyFile
)
{
    writeConfig("");

    ConfigLoader loader;

    Config config = loader.load(testFile);

    EXPECT_FALSE(config.contains("anything"));
}


// ----------------------------------------------------------
// String Values
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    LoadsStringValue
)
{
    writeConfig(
        "name = packetforge\n"
    );

    ConfigLoader loader;

    Config config = loader.load(testFile);

    ASSERT_TRUE(config.contains("name"));

    const ConfigValue* value =
        config.get("name");

    ASSERT_NE(value, nullptr);

    EXPECT_TRUE(value->isString());
    EXPECT_EQ(value->asString(), "packetforge");
}


// ----------------------------------------------------------
// Integer Values
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    LoadsIntegerValue
)
{
    writeConfig(
        "port = 8080\n"
    );

    ConfigLoader loader;

    Config config = loader.load(testFile);

    ASSERT_TRUE(config.contains("port"));

    const ConfigValue* value =
        config.get("port");

    ASSERT_NE(value, nullptr);

    EXPECT_TRUE(value->isInteger());
    EXPECT_EQ(value->asInteger(), 8080);
}


// ----------------------------------------------------------
// Boolean Values
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    LoadsBooleanTrue
)
{
    writeConfig(
        "enabled = true\n"
    );

    ConfigLoader loader;

    Config config = loader.load(testFile);

    ASSERT_TRUE(config.contains("enabled"));

    const ConfigValue* value =
        config.get("enabled");

    ASSERT_NE(value, nullptr);

    EXPECT_TRUE(value->isBoolean());
    EXPECT_TRUE(value->asBoolean());
}


TEST_F(
    ConfigLoaderTest,
    LoadsBooleanFalse
)
{
    writeConfig(
        "enabled = false\n"
    );

    ConfigLoader loader;

    Config config = loader.load(testFile);

    ASSERT_TRUE(config.contains("enabled"));

    const ConfigValue* value =
        config.get("enabled");

    ASSERT_NE(value, nullptr);

    EXPECT_TRUE(value->isBoolean());
    EXPECT_FALSE(value->asBoolean());
}


// ----------------------------------------------------------
// Multiple Values
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    LoadsMultipleValues
)
{
    writeConfig(
        "name = packetforge\n"
        "port = 8080\n"
        "enabled = true\n"
    );

    ConfigLoader loader;

    Config config = loader.load(testFile);

    ASSERT_TRUE(config.contains("name"));
    ASSERT_TRUE(config.contains("port"));
    ASSERT_TRUE(config.contains("enabled"));

    EXPECT_EQ(
        config.get("name")->asString(),
        "packetforge"
    );

    EXPECT_EQ(
        config.get("port")->asInteger(),
        8080
    );

    EXPECT_TRUE(
        config.get("enabled")->asBoolean()
    );
}


// ----------------------------------------------------------
// Whitespace
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    IgnoresWhitespaceAroundKeyAndValue
)
{
    writeConfig(
        "   port   =   8080   \n"
    );

    ConfigLoader loader;

    Config config = loader.load(testFile);

    ASSERT_TRUE(config.contains("port"));

    EXPECT_EQ(
        config.get("port")->asInteger(),
        8080
    );
}


// ----------------------------------------------------------
// Empty Lines
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    IgnoresEmptyLines
)
{
    writeConfig(
        "\n"
        "\n"
        "port = 8080\n"
        "\n"
    );

    ConfigLoader loader;

    Config config = loader.load(testFile);

    ASSERT_TRUE(config.contains("port"));

    EXPECT_EQ(
        config.get("port")->asInteger(),
        8080
    );
}


// ----------------------------------------------------------
// Comments
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    IgnoresCommentLines
)
{
    writeConfig(
        "# PacketForge configuration\n"
        "port = 8080\n"
        "# another comment\n"
    );

    ConfigLoader loader;

    Config config = loader.load(testFile);

    ASSERT_TRUE(config.contains("port"));

    EXPECT_EQ(
        config.get("port")->asInteger(),
        8080
    );

    EXPECT_FALSE(
        config.contains("# PacketForge configuration")
    );
}


// ----------------------------------------------------------
// Negative Integer
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    LoadsNegativeInteger
)
{
    writeConfig(
        "timeout = -10\n"
    );

    ConfigLoader loader;

    Config config = loader.load(testFile);

    ASSERT_TRUE(config.contains("timeout"));

    EXPECT_TRUE(
        config.get("timeout")->isInteger()
    );

    EXPECT_EQ(
        config.get("timeout")->asInteger(),
        -10
    );
}


// ----------------------------------------------------------
// Missing File
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    LoadingMissingFileThrows
)
{
    ConfigLoader loader;

    EXPECT_THROW(
        loader.load(
            "build/does_not_exist.conf"
        ),
        std::exception
    );
}


// ----------------------------------------------------------
// Malformed Configuration
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    MalformedLineThrows
)
{
    writeConfig(
        "this is not valid configuration\n"
    );

    ConfigLoader loader;

    EXPECT_THROW(
        loader.load(testFile),
        std::exception
    );
}


// ----------------------------------------------------------
// Missing Key
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    MissingKeyThrows
)
{
    writeConfig(
        "= value\n"
    );

    ConfigLoader loader;

    EXPECT_THROW(
        loader.load(testFile),
        std::exception
    );
}


// ----------------------------------------------------------
// Missing Value
// ----------------------------------------------------------

TEST_F(
    ConfigLoaderTest,
    MissingValueThrows
)
{
    writeConfig(
        "port =\n"
    );

    ConfigLoader loader;
   
    EXPECT_THROW(
        loader.load(testFile),
        std::exception
    );
}

} // namespace packetforge::config
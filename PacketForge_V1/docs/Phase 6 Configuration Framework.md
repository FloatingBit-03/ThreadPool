PacketForge — Phase 6 Configuration Framework
Requirements & API Freeze Specification

1. Purpose
The configuration framework provides a common mechanism for PacketForge applications to obtain runtime configuration.
The framework must initially remain:

lightweight
dependency-free
easy to test
type-safe at the API boundary
independent of Client and Server implementations
extensible toward more sophisticated configuration systems

The framework must not become responsible for application behavior.

Its job is:

Configuration Sources
        |
        v
Configuration Framework
        |
        v
Validated Configuration
        |
        v
Application

2. Primary Design Goal
The most important requirement is: Application code must not depend on the configuration storage format.

For example, the server application should be able to write:
auto port =
    config.get<std::uint16_t>("server.port");

without knowing whether the value originated from:

default
configuration file
environment variable
command line
JSON
YAML
TOML
remote configuration

This gives us future extensibility without coupling the application to a particular format.

3. V1 Scope
The first version will support:

Configuration values
string
boolean
signed integer
unsigned integer
floating-point
Configuration keys

Hierarchical keys such as:

server.address
server.port
server.backlog

client.server_address
client.server_port
Configuration sources

V1:

Built-in defaults
Simple configuration file
Configuration operations
set
get
contains
remove
load
Validation

V1 should support basic validation such as:

required key
numeric range
valid value type

4. Explicitly Out of Scope for V1

We should deliberately not implement these yet:
JSON parser
YAML parser
TOML parser
XML
Remote configuration
Database configuration
Hot reload
Configuration watching
Encrypted configuration
Schema language
Complex expression evaluation
Dynamic configuration updates
Plugin-based configuration

They may be future extensions. This keeps Phase 6 manageable.

5. Configuration Architecture
The frozen architecture is:

                         Application
                              |
                              v
                    +-------------------+
                    |      Config       |
                    |   Public API      |
                    +-------------------+
                              |
                    +---------+---------+
                    |                   |
                    v                   v
              ConfigValue         Configuration
                                  State/Storage
                    ^
                    |
             +------+------+
             |             |
             v             v
          Defaults     File Loader

Later:

                 Config
                   |
       +-----------+-----------+
       |           |           |
       v           v           v
     File         ENV         CLI
       |
       v
    JSON/YAML/TOML

The public Config API remains stable.

6. Frozen Module Structure
Recommend the following structure:

include/
└── config/
    ├── config.hpp
    ├── config_value.hpp
    └── config_loader.hpp

src/
└── config/
    ├── config.cpp
    ├── config_value.cpp
    └── config_loader.cpp

tests/
└── unit/
    ├── test_config.cpp
    ├── test_config_value.cpp
    └── test_config_loader.cpp

We should not create additional configuration classes until there is a requirement for them.

7. ConfigValue
ConfigValue represents one configuration value.
Conceptually:

ConfigValue
     |
     +-- bool
     +-- signed integer
     +-- unsigned integer
     +-- floating point
     +-- string

The implementation should use a C++20-friendly type-safe representation, most naturally: std::variant
rather than a manually managed union.

Conceptually:
using Value = std::variant<
    bool,
    std::int64_t,
    std::uint64_t,
    double,
    std::string
>;

This gives us a controlled and predictable V1 type system.

8. Why Not Store Every Integer Type?
We should not create:

int8
int16
int32
int64
uint8
uint16
uint32
uint64

as eight independent configuration types.

Instead, internally use:

signed integer   -> int64_t
unsigned integer -> uint64_t

The public API can convert appropriately.

For example:
auto port =
    config.get<std::uint16_t>("server.port");

The stored value can still be: uint64_t ,with range checking during retrieval.

9. Frozen ConfigValue API
The conceptual public interface should be:
class ConfigValue
{
public:

    ConfigValue();

    ConfigValue(bool value);

    ConfigValue(std::int64_t value);

    ConfigValue(std::uint64_t value);

    ConfigValue(double value);

    ConfigValue(const std::string& value);

    ConfigValue(std::string&& value);


    template<typename T>
    bool is() const noexcept;


    template<typename T>
    common::Result<T> get() const;


    const Value& raw() const noexcept;
};

However, there is an important point here: We should not introduce Result<T> yet unless PacketForge already has a generic result abstraction.
Existing project has common::Error, so it should first inspect the existing error design before freezing the exact return type.
Therefore the semantic API is frozen now, while the exact error-return syntax should reuse the existing common abstraction.

10. Config
Config is the primary user-facing configuration object. It owns the current configuration state.

Conceptually:
class Config
{
public:

    Config();

    ~Config();

    Config(const Config&) = default;
    Config& operator=(const Config&) = default;

    Config(Config&&) noexcept;
    Config& operator=(Config&&) noexcept;


    common::Error set(
        const std::string& key,
        ConfigValue value
    );


    bool contains(
        const std::string& key
    ) const noexcept;


    common::Error remove(
        const std::string& key
    );


    common::Error load(
        const std::string& filename
    );


    template<typename T>
    common::Error get(
        const std::string& key,
        T& value
    ) const;


    void clear() noexcept;
};

This is the core API boundary.

11. Why get(key, value) Instead of Returning a Value?
I recommend that our V1 API initially follow the project's existing error-oriented style:

std::uint16_t port{};

auto error =
    config.get(
        "server.port",
        port
    );

if (!error.ok())
{
    // handle error
}

rather than immediately introducing:

auto port =
    config.get<std::uint16_t>("server.port");

The latter is elegant, but it requires us to introduce a generic result/expected abstraction.

Since PacketForge already has: common::Error
we should avoid unnecessarily expanding the common module during this milestone.

Later we can add: common::Result<T>
if the project genuinely benefits from it.

12. Key Rules
Configuration keys follow this format: component.property
Examples:

server.address
server.port
server.backlog

client.server_address
client.server_port

Allowed characters for V1:

a-z
A-Z
0-9
_
-
.

Keys should not:

be empty
start with .
end with .
contain consecutive ..

Invalid:

.server.port
server.port.
server..port
13. Case Sensitivity

Configuration keys are case-sensitive.

Therefore: server.port
and: Server.Port
are different keys. This avoids hidden transformations and makes behavior deterministic.

14. Configuration File Format
The V1 file format will deliberately be simple:
key=value

Example:

server.address=127.0.0.1
server.port=9000
server.backlog=128

Comments: # PacketForge server configuration

server.address=127.0.0.1
server.port=9000

Blank lines are ignored.
Whitespace surrounding keys and values should be ignored: server.port = 9000, is equivalent to: server.port=9000

15. Configuration File Parser Rules
V1 parser behavior:blank line-> ignore

line beginning with #  -> ignore

key=value  -> parse

anything else  -> error

Example:
server.port=9000       OK
# comment              OK
server.port            ERROR
=9000                  ERROR

16. Value Parsing
The loader should infer the basic type.
Examples:
true
false
become:bool

Integer:9000
becomes an integer.

Floating point: 3.14
becomes: double

Everything else becomes a string.

For example: server.address=127.0.0.1
should remain a string.

This is important because an IP address is not a numeric configuration value.

17. Quoting
V1 should keep quoting deliberately simple.

We can support: server.address="127.0.0.1"
but we should not implement a full escape-language parser yet.

No complicated rules such as:
nested quotes
unicode escapes
multiline strings
shell escaping

Those belong to a future format/parser.

18. Configuration Precedence
This is one of the most important things to freeze now.

The long-term precedence is:

             Highest priority
                    |
                    v
             Command Line
                    |
             Environment
                    |
             Config File
                    |
             Built-in Defaults
                    |
                    v
             Lowest priority

V1 implementation:

Config File
     |
     v
Defaults

Therefore:

Default: server.port = 9000

File: server.port = 8080

results in: server.port = 8080

We design for future precedence now, but we don't implement CLI/environment yet.

19. Defaults
Defaults should be established by the application, not by the generic Config class.

For example:

Config config;

config.set(
    "server.address",
    "127.0.0.1"
);

config.set(
    "server.port",
    std::uint16_t{9000}
);

The configuration framework should not contain: server.port = 9000
because that is application policy.

This distinction is critical.

20. Server Configuration
The Server application will eventually define something like:

server.address
server.port
server.backlog

For example:

server.address=127.0.0.1
server.port=9000
server.backlog=128

The configuration framework does not know what these mean. It simply stores them.

21. Client Configuration
The Client application may define:
client.server_address
client.server_port

Example:
client.server_address=127.0.0.1
client.server_port=9000

Again, the configuration framework remains application-independent.

22. Validation Responsibility
We should distinguish:
Parsing
"9000"

is syntactically an integer.

Application validation
server.port must be 1-65535

The second rule belongs to the application/configuration schema, not the generic parser.

Therefore:
ConfigLoader
    |
    v
Parsing
    |
    v
Config
    |
    v
Application Validation

For V1, validation can initially be performed in the application layer. A dedicated ConfigValidator can be introduced later.

23. Error Model
The configuration system must never silently ignore configuration errors.

Examples:

File cannot be opened
Invalid configuration line
Duplicate malformed entry
Invalid value conversion
Missing required key
Type mismatch
Numeric overflow

must produce an appropriate common::Error.
It should reuse the project's existing common::ErrorCode model rather than creating a completely independent configuration error mechanism.
Before implementation, we should therefore add only the minimum required configuration-specific error codes if they don't already exist.

24. Duplicate Keys
This needs to be explicit.
V1 rule: Later values override earlier values.

Example:

server.port=9000
server.port=8000

results in:

server.port=8000

This also naturally supports future precedence handling.

25. Thread Safety
V1 configuration objects are not thread-safe.
Expected lifecycle:

Load configuration
        |
        v
Validate configuration
        |
        v
Start application
        |
        v
Read configuration

Configuration should normally become immutable from the application's point of view after startup.

We do not need mutexes in the first implementation. This keeps the framework lightweight.

26. Configuration Lifecycle
The application lifecycle should eventually be:

Application Start
       |
       v
Create Config
       |
       v
Set Defaults
       |
       v
Load Configuration File
       |
       v
Validate Configuration
       |
       v
Create Client/Server
       |
       v
Start Application

This is the correct place for configuration.

27. Configuration Must Not Leak Into Core Modules
This is a hard architectural boundary.

Do not change: Server::start(...)

into something like: Server::start(Config&)

The Server API should remain:

server.start(
    address,
    port
);

The application translates configuration into server parameters.

Therefore:

Config
  |
  v
server_main
  |
  +---- address
  +---- port
  |
  v
Server

This keeps the core networking library reusable.

28. Future Extension Point
The framework should eventually support:

class ConfigSource
{
public:
    virtual ~ConfigSource() = default;

    virtual common::Error load(
        Config& config
    ) = 0;
};

Potential implementations:

FileConfigSource
EnvironmentConfigSource
CommandLineConfigSource
JsonConfigSource
YamlConfigSource
TomlConfigSource

But do not implement this abstraction in the first iteration unless the initial implementation actually benefits from it.
The architectural direction now.

29. Future API

The eventual system could look like:

Config config;

config.defaults(...);

config.load(file);

config.load(environment);

config.load(commandLine);

config.validate(schema);

with precedence automatically handled.

The important point is that the application API remains stable.

30. Frozen Design Principles
These principles should now be treated as constraints during implementation.

Principle 1
Configuration is independent of Server and Client.

Principle 2
Application owns configuration policy.

Principle 3
Config owns configuration state.

Principle 4
Loader owns parsing.

Principle 5
Configuration values are type-safe.

Principle 6
V1 remains dependency-free.

Principle 7
V1 supports simple files, not complex formats.

Principle 8
Future configuration sources must be addable without changing application code.

Principle 9
Configuration is loaded during application startup.

Principle 10
Core networking classes must not depend on the configuration module.

31. Frozen Dependency Direction
The dependency direction is:

                    Application
                         |
             +-----------+-----------+
             |                       |
             v                       v
        Configuration             Server
             |
             v
         Common/Error

Not:
Server
   |
   v
Configuration

and not:
Configuration
   |
   v
Server

The configuration module should only depend on lower-level common functionality.

32. Final V1 Architecture
The resulting Phase 6 structure will eventually become:

PacketForge
│
├── common
│   ├── Error
│   ├── Logger
│   ├── Buffer
│   └── Endian
│
├── network
│   ├── Socket
│   ├── Connection
│   └── Transport
│
├── protocol
│   ├── Packet
│   ├── Encoder
│   └── Decoder
│
├── client
│   └── Client
│
├── server
│   └── Server
│
└── config
    ├── Config
    ├── ConfigValue
    └── ConfigLoader

Then:
                 server_main
                     |
          +----------+----------+
          |                     |
          v                     v
       Config                 Logger
          |
          v
   Validated Values
          |
          v
        Server
          |
          v
       Network

and:

                 client_main
                     |
          +----------+----------+
          |                     |
          v                     v
       Config                 Logger
          |
          v
   Validated Values
          |
          v
        Client
          |
          v
       Network
33. Configuration Milestone Breakdown
Now that the design is frozen, implementation should proceed in small milestones.

C1 — ConfigValue
Implement:

bool
int64
uint64
double
string

and type-safe access.

C2 — Config
Implement:

set
get
contains
remove
clear

C3 — Key Validation
Implement:
valid keys
invalid keys

C4 — File Loader
Implement:

key=value
comments
blank lines
whitespace
duplicate keys

C5 — Error Handling
Integrate configuration errors with:

common::Error

C6 — Unit Tests
Test the framework independently.

C7 — Application Integration
Only after C1–C6 are stable:

server_main
client_main

begin consuming configuration.

34. The API Freeze Point
At this point, I recommend we freeze the semantic API and stop changing its architecture while implementing C1–C6.

The only thing I would verify against the existing codebase before writing config.hpp is the exact shape of common::Error and ErrorCode, because configuration errors should integrate with what you already built rather than introducing a second error mechanism.

So the development route from here is:

             DESIGN COMPLETE
                    |
                    v
             ┌─────────────┐
             │ C1          │
             │ ConfigValue │
             └──────┬──────┘
                    |
                    v
             ┌─────────────┐
             │ C2          │
             │ Config      │
             └──────┬──────┘
                    |
                    v
             ┌─────────────┐
             │ C3          │
             │ Key Rules   │
             └──────┬──────┘
                    |
                    v
             ┌─────────────┐
             │ C4          │
             │ File Loader │
             └──────┬──────┘
                    |
                    v
             ┌─────────────┐
             │ C5          │
             │ Errors      │
             └──────┬──────┘
                    |
                    v
             ┌─────────────┐
             │ C6          │
             │ Unit Tests  │
             └──────┬──────┘
                    |
                    v
          CONFIGURATION V1 COMPLETE
                    |
                    v
             Server / Client
             Application
             Integration
             
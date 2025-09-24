#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <random>
#include <chrono>
#include <array>
#include <cstring>
#include <mutex>
#include <atomic>

// ===== Kleine SHA-1 implementatie =====
class SHA1 {
public:
    SHA1() { reset(); }

    void update(const std::string &s) {
        update(reinterpret_cast<const uint8_t*>(s.c_str()), s.size());
    }

    void update(const uint8_t *data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            m_block[m_blockByteIndex++] = data[i];
            m_byteCount++;
            if (m_blockByteIndex == 64) {
                processBlock();
                m_blockByteIndex = 0;
            }
        }
    }

    std::array<uint8_t, 20> digest() {
        uint64_t totalBits = m_byteCount * 8;

        // Append '1' bit
        update((uint8_t*)"\x80", 1);

        // Pad with zeros until 56 bytes mod 64
        uint8_t zero = 0;
        while (m_blockByteIndex != 56) {
            update(&zero, 1);
        }

        // Append length in bits
        uint8_t lengthBytes[8];
        for (int i = 0; i < 8; ++i) {
            lengthBytes[7 - i] = static_cast<uint8_t>(totalBits >> (i * 8));
        }
        update(lengthBytes, 8);

        // Output
        std::array<uint8_t, 20> result;
        for (int i = 0; i < 5; ++i) {
            result[i*4]     = static_cast<uint8_t>(m_h[i] >> 24);
            result[i*4 + 1] = static_cast<uint8_t>(m_h[i] >> 16);
            result[i*4 + 2] = static_cast<uint8_t>(m_h[i] >> 8);
            result[i*4 + 3] = static_cast<uint8_t>(m_h[i]);
        }
        return result;
    }

private:
    uint32_t m_h[5];
    uint8_t m_block[64];
    size_t m_blockByteIndex;
    uint64_t m_byteCount;

    void reset() {
        m_h[0] = 0x67452301;
        m_h[1] = 0xEFCDAB89;
        m_h[2] = 0x98BADCFE;
        m_h[3] = 0x10325476;
        m_h[4] = 0xC3D2E1F0;
        m_blockByteIndex = 0;
        m_byteCount = 0;
    }

    void processBlock() {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i) {
            w[i] = (m_block[i*4] << 24) | (m_block[i*4 + 1] << 16) |
                   (m_block[i*4 + 2] << 8) | (m_block[i*4 + 3]);
        }
        for (int i = 16; i < 80; ++i) {
            uint32_t val = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
            w[i] = (val << 1) | (val >> 31);
        }

        uint32_t a = m_h[0];
        uint32_t b = m_h[1];
        uint32_t c = m_h[2];
        uint32_t d = m_h[3];
        uint32_t e = m_h[4];

        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
            e = d;
            d = c;
            c = (b << 30) | (b >> 2);
            b = a;
            a = temp;
        }

        m_h[0] += a;
        m_h[1] += b;
        m_h[2] += c;
        m_h[3] += d;
        m_h[4] += e;
    }
};

// ===== Helper functies =====
std::string formatUUID(const std::array<unsigned char, 16>& bytes) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < bytes.size(); ++i) {
        oss << std::setw(2) << (int)bytes[i];
        if (i == 3 || i == 5 || i == 7 || i == 9) oss << "-";
    }
    return oss.str();
}

// Thread-local RNG helper
// Thread-local RNG helper
static std::mt19937_64& tls_rng() {
    thread_local static std::mt19937_64 rng([]{
        std::random_device rd;
        auto seed = (static_cast<uint64_t>(rd()) << 32) ^ static_cast<uint64_t>(rd());
        return std::mt19937_64(seed);
    }());
    return rng;
}


// Random UUID v4 (thread-safe via thread_local RNG)
std::string uuid_v4() {
    std::uniform_int_distribution<uint32_t> dis32(0, 0xFFFFFFFFu);

    std::array<unsigned char, 16> bytes;
    uint32_t r1 = dis32(tls_rng());
    uint32_t r2 = dis32(tls_rng());
    uint32_t r3 = dis32(tls_rng());
    uint32_t r4 = dis32(tls_rng());

    bytes[0] = (r1 >> 24) & 0xFF; bytes[1] = (r1 >> 16) & 0xFF; bytes[2] = (r1 >> 8) & 0xFF; bytes[3] = r1 & 0xFF;
    bytes[4] = (r2 >> 24) & 0xFF; bytes[5] = (r2 >> 16) & 0xFF; bytes[6] = (r2 >> 8) & 0xFF; bytes[7] = r2 & 0xFF;
    bytes[8] = (r3 >> 24) & 0xFF; bytes[9] = (r3 >> 16) & 0xFF; bytes[10]= (r3 >> 8) & 0xFF; bytes[11]= r3 & 0xFF;
    bytes[12]= (r4 >> 24) & 0xFF; bytes[13]= (r4 >> 16) & 0xFF; bytes[14]= (r4 >> 8) & 0xFF; bytes[15]= r4 & 0xFF;

    // Versie 4 en variant bits
    bytes[6] = (bytes[6] & 0x0F) | 0x40;
    bytes[8] = (bytes[8] & 0x3F) | 0x80;

    return formatUUID(bytes);
}

// Naam-gebaseerde UUID v5 (thread-safe: geen gedeelde staat; RNG niet nodig)
std::string uuid_v5(const std::string& namespace_uuid, const std::string& name) {
    std::array<unsigned char, 16> ns_bytes{};
    // Robuust parsen van UUID-hex met streepjes
    size_t bi = 0;
    unsigned int val = 0;
    int nybble = -1; // -1 = none, 0 = high nibble stored
    for (char c : namespace_uuid) {
        if (c == '-') continue;
        if (!isxdigit(static_cast<unsigned char>(c))) continue;
        unsigned int d = (c <= '9') ? (c - '0') :
                         (c <= 'F') ? (10 + (c - 'A')) :
                                      (10 + (c - 'a'));
        if (nybble < 0) {
            val = (d << 4);
            nybble = 0;
        } else {
            val |= d;
            if (bi < 16) ns_bytes[bi++] = static_cast<unsigned char>(val);
            nybble = -1;
        }
    }
    if (bi != 16) {
        // fallback: zero namespace on parse failure
        ns_bytes.fill(0);
    }

    std::string input(reinterpret_cast<char*>(ns_bytes.data()), ns_bytes.size());
    input += name;

    SHA1 sha;
    sha.update(input);
    auto hash = sha.digest();

    std::array<unsigned char, 16> bytes;
    std::memcpy(bytes.data(), hash.data(), 16);

    // Versie 5 en variant bits
    bytes[6] = (bytes[6] & 0x0F) | 0x50;
    bytes[8] = (bytes[8] & 0x3F) | 0x80;

    return formatUUID(bytes);
}

// ===== RFC 4122 v1 time-based UUID (thread-safe) =====
std::string uuid_v1() {
    // Globale gedeelde staat voor v1
    static std::once_flag init_flag;
    static std::mutex mtx;
    static uint64_t node = 0;                 // 48 bits, multicast-bit aan als random
    static uint16_t clock_seq = 0;            // 14 bits
    static uint64_t last_uuid_time = 0;       // in 100ns ticks sinds 1582-10-15
    static int64_t last_unix_ns = 0;          // detectie klok-terugloop

    // Init eenmaal
    std::call_once(init_flag, [] {
        std::uniform_int_distribution<uint64_t> dist64(0, 0xFFFFFFFFFFFFFFFFULL);
        std::uniform_int_distribution<uint16_t> dist16(0, 0xFFFF);
        auto& rng = tls_rng();

        node = dist64(rng) & 0x0000FFFFFFFFFFFFULL;
        node |= 0x010000000000ULL; // multicast-bit (random node)
        clock_seq = dist16(rng) & 0x3FFF; // 14 bits
        last_uuid_time = 0;
        last_unix_ns = 0;
    });

    using namespace std::chrono;
    const uint64_t UUID_EPOCH_START = 0x01B21DD213814000ULL; // offset 1582->1970 in 100ns
    const auto now = system_clock::now();
    const int64_t unix_ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();
    uint64_t now_ticks = UUID_EPOCH_START + static_cast<uint64_t>(unix_ns / 100);

    uint64_t ts;
    {
        std::lock_guard<std::mutex> lock(mtx);

        // Detecteer klok-terugloop en verhoog clock_seq
        if (unix_ns < last_unix_ns) {
            clock_seq = (clock_seq + 1) & 0x3FFF;
        }
        last_unix_ns = unix_ns;

        // Zorg voor monotone timestamp en sub-tick handling
        if (now_ticks <= last_uuid_time) {
            ts = last_uuid_time + 1; // zelfde of kleinere tick: schuif 1 stap
        } else {
            ts = now_ticks;
        }
        last_uuid_time = ts;
    }

    // Velden samenstellen volgens RFC 4122
    uint32_t time_low   = static_cast<uint32_t>(ts & 0xFFFFFFFFULL);
    uint16_t time_mid   = static_cast<uint16_t>((ts >> 32) & 0xFFFFULL);
    uint16_t time_hi    = static_cast<uint16_t>((ts >> 48) & 0x0FFFULL); // 12 bits
    time_hi |= 0x1000; // versie 1

    uint8_t clock_seq_hi = static_cast<uint8_t>((clock_seq >> 8) & 0x3F);
    clock_seq_hi |= 0x80; // variant RFC 4122
    uint8_t clock_seq_lo = static_cast<uint8_t>(clock_seq & 0xFF);

    std::array<unsigned char, 16> bytes;

    // time_low (big-endian bytes)
    bytes[0] = (time_low >> 24) & 0xFF;
    bytes[1] = (time_low >> 16) & 0xFF;
    bytes[2] = (time_low >> 8)  & 0xFF;
    bytes[3] = (time_low)       & 0xFF;

    // time_mid
    bytes[4] = (time_mid >> 8) & 0xFF;
    bytes[5] = (time_mid)      & 0xFF;

    // time_hi_and_version
    bytes[6] = (time_hi >> 8) & 0xFF;
    bytes[7] = (time_hi)      & 0xFF;

    // clock_seq_hi_and_reserved + clock_seq_low
    bytes[8] = clock_seq_hi;
    bytes[9] = clock_seq_lo;

    // node (48 bits)
    for (int i = 0; i < 6; ++i) {
        bytes[10 + i] = static_cast<unsigned char>((node >> ((5 - i) * 8)) & 0xFF);
    }

    return formatUUID(bytes);
}

// ===== Main demo =====
int main() {
    std::cout << "Random UUID v4:         " << uuid_v4() << "\n";
    std::cout << "Naam-gebaseerde v5:     " 
              << uuid_v5("6ba7b810-9dad-11d1-80b4-00c04fd430c8", "MijnNaam") << "\n";
    std::cout << "Tijd-gebaseerde v1:     " << uuid_v1() << "\n";
    return 0;
}

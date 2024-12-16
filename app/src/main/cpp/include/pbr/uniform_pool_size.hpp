#ifndef PBR_UNIFORM_POOL_SIZE_HPP
#define PBR_UNIFORM_POOL_SIZE_HPP


namespace pbr {

enum class eUniformPoolSize : size_t
{
    Nanoscopic_64KB = 64U,
    Microscopic_1M [[maybe_unused]] = 1024U,
    Tiny_4M = 4096U,
    Small_8M [[maybe_unused]] = 8192U,
    Medium_16M [[maybe_unused]] = 16384U,
    Big_32M [[maybe_unused]] = 32768U,
    Huge_64M = 65536U
};


} // namespace pbr


#endif // PBR_UNIFORM_POOL_SIZE_HPP

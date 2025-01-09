#pragma once

#include <cstdint>
#include <fstream>

#include "Bit_reader.hpp"

/**
 * @brief Decodes a UTF-8 encoded number from a file stream.
 *
 * This function reads bytes from the provided file stream and decodes them
 * into a single UTF-8 code point. It handles multi-byte UTF-8 sequences and
 * validates the encoding.
 *
 * @param file_stream The input file stream to read from.
 * @return The decoded UTF-8 code point as a 64-bit unsigned integer.
 * @throws std::runtime_error If the UTF-8 encoding is invalid.
 */
uint64_t decode_utf8(std::ifstream &file_stream);

/**
 * @brief Decodes a unary encoded integer from a bit reader.
 *
 * This function reads bits from the provided bit reader and decodes them
 * into a unary encoded integer. Unary encoding represents a number n as n
 * zero bits followed by a one bit.
 *
 * @param reader The bit reader to read from.
 * @return The decoded unary encoded integer as a 64-bit unsigned integer.
 */
uint64_t decode_unary(mc::Bit_reader<std::ifstream> &reader);

/**
 * @brief Decodes an Rice encoded integer from a bit reader.
 *
 * This function reads bits from the provided bit reader and decodes them
 * into a signed integer.
 *
 * @param reader The bit reader to read from.
 * @return The decoded Rice encoded integer as a 64-bit signed integer.
 */
int64_t decode_and_unfold_rice(uint8_t rice_parameter, mc::Bit_reader<std::ifstream> &reader);

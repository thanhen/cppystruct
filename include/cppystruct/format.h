#pragma once
#include <cppystruct/string.h>
#include <iostream>
namespace pystruct {

constexpr bool isFormatMode(char formatChar)
{
	return formatChar == '<' || formatChar == '>' || formatChar == '!'
			   				 || formatChar == '=' || formatChar == '@';
}

constexpr bool isFormatChar(char formatChar)
{
	return isFormatMode(formatChar) || formatChar == 'x' || formatChar == 'b'
		|| formatChar == 'B' || formatChar == 'c' || formatChar == 's'
		|| formatChar == 'h' || formatChar == 'H' || formatChar == 'i'
		|| formatChar == 'I' || formatChar == 'l' || formatChar == 'L'
		|| formatChar == 'q' || formatChar == 'Q' || formatChar == 'f'
		|| formatChar == 'd' 
		|| internal::isDigit(formatChar);
}


// Specifying the format mode
template <char FormatChar>
struct FormatMode
{
	static_assert(isFormatMode(FormatChar), "Invalid Format Mode passed");

	static constexpr bool isBigEndian() { return false; };
	static constexpr bool shouldPad() { return false; };
};

#define SET_FORMAT_MODE(mode, padding, bigEndian) \
    template <> struct FormatMode<mode> { \
		static constexpr bool isBigEndian() { return bigEndian; }; \
		static constexpr bool shouldPad() { return padding; }; \
	}

SET_FORMAT_MODE('@', true, false);
SET_FORMAT_MODE('>', false, true);
SET_FORMAT_MODE('!', false, true);

constexpr bool doesFormatAlign(size_t size) 
{
	return size > 1;
}


// Specifying the Big Endian format
template <char FormatChar>
struct BigEndianFormat
{
	static_assert(isFormatChar(FormatChar), "Invalid Format Char passed");
	static constexpr size_t size() { return 0; }
};

#define SET_FORMAT_CHAR(ch, s) \
    template <> struct BigEndianFormat<ch> { \
		static constexpr size_t size() { return s; } \
	}

SET_FORMAT_CHAR('x', 1);
SET_FORMAT_CHAR('b', 1);
SET_FORMAT_CHAR('B', 1);
SET_FORMAT_CHAR('c', 1);
SET_FORMAT_CHAR('s', 1);

// Pascal strings are not supported ideologically
//SET_FORMAT_CHAR('p', 1); 

SET_FORMAT_CHAR('h', 2);
SET_FORMAT_CHAR('H', 2);
SET_FORMAT_CHAR('i', 4);
SET_FORMAT_CHAR('I', 4);
SET_FORMAT_CHAR('l', 4);
SET_FORMAT_CHAR('L', 4);
SET_FORMAT_CHAR('q', 8);
SET_FORMAT_CHAR('Q', 8);
SET_FORMAT_CHAR('f', 4);
SET_FORMAT_CHAR('d', 8);


template <typename Fmt>
constexpr auto getFormatMode(Fmt&&)
{
	// First format char is a format mode
	if constexpr(isFormatMode(Fmt::at(0))) {
		constexpr auto firstChar = Fmt::at(0);
		return FormatMode<firstChar>{};
	} else {
		return FormatMode<'@'>{};
	}
}

template <typename Fmt>
constexpr auto countItems(Fmt&&)
{
	size_t itemCount = 0;

	size_t multiplier = 1;
	for(size_t i = 0; i < Fmt::size(); i++) {
		auto currentChar = Fmt::at(i);
		if(i == 0 && isFormatMode(currentChar)) {
			continue;
		}

		if (internal::isDigit(currentChar)) {
			if (multiplier == 1) {
				multiplier = currentChar - '0';
			} else {
				multiplier = multiplier * 10 + (currentChar - '0');
			}

			continue;
		}

		itemCount += multiplier;
		multiplier = 1;
	}

	return itemCount;
}

template <size_t Item, typename Fmt, size_t CurrentItem=0, size_t CurrentI=0, size_t Multiplier=1, size_t... Is>
constexpr char getTypeOfItem(std::index_sequence<Is...>)
{
	constexpr char chars[] = { Fmt::at(Is)... };

	if constexpr (internal::isDigit(Fmt::at(CurrentI))) {
		constexpr auto numberAndIndex = internal::consumeNumber(chars, CurrentI);
		return getTypeOfItem<Item, Fmt, CurrentItem, numberAndIndex.second, numberAndIndex.first>(std::index_sequence<Is...>{});
	}

	if constexpr ((Item >= CurrentItem) && (Item < (CurrentItem + Multiplier))) {
		constexpr char currentChar = Fmt::at(CurrentI);
		return currentChar;
	} else {
		return getTypeOfItem<Item, Fmt, CurrentItem+Multiplier, CurrentI+1>(std::index_sequence<Is...>{});
	}
}

template <size_t Index, typename Fmt>
constexpr auto getTypeOfItem(Fmt&&)
{
	return getTypeOfItem<Index, Fmt>(std::make_index_sequence<Fmt::size()>());
}


} // namespace pystruct
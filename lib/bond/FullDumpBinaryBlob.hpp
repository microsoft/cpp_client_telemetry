//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

// Not using std::vector<> directly to be able to provide
// custom formatter which dumps the full data.
class FullDumpBinaryBlob : public std::vector<uint8_t>
{
  public:
    FullDumpBinaryBlob() {}
    FullDumpBinaryBlob(std::initializer_list<uint8_t> values) : std::vector<uint8_t>(values) {}
};

static void PrintTo(FullDumpBinaryBlob const& value, ::std::ostream* os)
{
    *os << value.size() << " bytes -";
    for (size_t addr = 0; addr < value.size(); addr += 32) {
        *os << "\n          \"";
        for (size_t i = 0; i < 32 && addr + i < value.size(); i++) {
            unsigned x = value[addr + i];
            *os << '\\' << 'x' << "0123456789ABCDEF"[x / 16] << "0123456789ABCDEF"[x % 16];
        }
        *os << '"';
    }
}

class FullDumpBinaryEqMatcher : public testing::MatcherInterface<FullDumpBinaryBlob const&> {
  public:
    FullDumpBinaryEqMatcher(FullDumpBinaryBlob const& expected)
      : m_expected(expected)
    {
    }

    FullDumpBinaryEqMatcher(FullDumpBinaryEqMatcher const&) = delete;

    virtual bool MatchAndExplain(FullDumpBinaryBlob const& actual, testing::MatchResultListener* listener) const override
    {
        if (actual == m_expected) {
            return true;
        }

        std::ostream* os = listener->stream();
        if (!os) {
            return false;
        }

        size_t ofs = 0;
        while (ofs < actual.size() && ofs < m_expected.size() && actual[ofs] == m_expected[ofs]) {
            ofs++;
        }
        size_t dofs = (ofs >= 8) ? ofs - 8 : 0;
        *os << "\n          which differ from offset " << ofs << " -\n";

        *os << "  Actual: ";
        if (dofs > 0) {
            *os << "... ";
        }
        if (dofs == ofs) {
            *os << ">>";
        }
        *os << '"';
        for (size_t i = dofs; i < dofs + 16 && i < actual.size(); i++) {
            if (i == ofs && ofs != 0) {
                *os << "\" >>\"";
            }
            unsigned x = actual[i];
            *os << '\\' << 'x' << "0123456789ABCDEF"[x / 16] << "0123456789ABCDEF"[x % 16];
        }
        *os << '"';
        if (dofs + 16 < actual.size()) {
            *os << " ...";
        }
        *os << '\n';

        *os << "Expected: ";
        if (dofs > 0) {
            *os << "    ";
        }
        if (ofs != 0) {
            *os << "   ";
        }
        for (size_t i = dofs; i < dofs + 16 && i < m_expected.size(); i++) {
            if (i < ofs) {
                *os << "    ";
                continue;
            }
            if (i == ofs) {
                *os << ">>\"";
            }
            unsigned x = m_expected[i];
            *os << '\\' << 'x' << "0123456789ABCDEF"[x / 16] << "0123456789ABCDEF"[x % 16];
        }
        *os << '"';

        return false;
    }

    virtual void DescribeTo(::std::ostream* os) const override
    {
        *os << "equals ";
        PrintTo(m_expected, os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const override
    {
        *os << "does not equal ";
        PrintTo(m_expected, os);
    }

  protected:
    FullDumpBinaryBlob m_expected;
};

inline testing::Matcher<FullDumpBinaryBlob const&> FullDumpBinaryEq(FullDumpBinaryBlob const& expected)
{
    return testing::MakeMatcher(new FullDumpBinaryEqMatcher(expected));
}


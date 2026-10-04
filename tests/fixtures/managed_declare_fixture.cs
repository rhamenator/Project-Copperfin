// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;

namespace Copperfin.ManagedDeclareFixture
{
    public static class Methods
    {
        private static int integerBoundaryEntryCount;

        public static int ReturnFortyTwo()
        {
            return 42;
        }

        public static int Add(int left, int right)
        {
            return left + right;
        }

        public static long WidenInt64(long value)
        {
            return value + 1L;
        }

        public static long ReturnInt64BeyondDouble()
        {
            return 9007199254740993L;
        }

        public static long PreserveInt64(long value)
        {
            return value;
        }

        public static ulong ReturnUInt64BeyondDouble()
        {
            return 18014398509481985UL;
        }

        public static long ReturnInt64Maximum()
        {
            return long.MaxValue;
        }

        public static ulong ReturnUInt64Maximum()
        {
            return ulong.MaxValue;
        }

        public static ulong ReturnUInt64AtSignedMaximum()
        {
            return 9223372036854775807UL;
        }

        public static double ReturnNaN()
        {
            return double.NaN;
        }

        public static int ResetIntegerBoundaryEntryCount()
        {
            integerBoundaryEntryCount = 0;
            return 0;
        }

        public static int IntegerBoundaryEntryCount()
        {
            return integerBoundaryEntryCount;
        }

        public static int EchoInt32Boundary(int value)
        {
            ++integerBoundaryEntryCount;
            return value;
        }

        public static long EchoInt64Boundary(long value)
        {
            ++integerBoundaryEntryCount;
            return value;
        }

        public static double WidenDouble(double value)
        {
            return value + 0.5;
        }

        public static float WidenSingle(float value)
        {
            return value + 0.25F;
        }

        public static string Echo(string value)
        {
            return value;
        }

        public static int ReturnDependencyValue()
        {
            return Copperfin.ManagedDeclareDependency.Values.Expected;
        }

        public static int ThrowAlways()
        {
            throw new InvalidOperationException("Copperfin managed DECLARE fixture failure");
        }
    }
}

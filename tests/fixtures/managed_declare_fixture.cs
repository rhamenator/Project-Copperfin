// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;

namespace Copperfin.ManagedDeclareFixture
{
    public static class Methods
    {
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

        public static double WidenDouble(double value)
        {
            return value + 0.5;
        }

        public static float WidenSingle(float value)
        {
            return value + 0.25F;
        }

        public static bool ReturnTrue()
        {
            return true;
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

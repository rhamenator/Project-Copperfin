// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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

        public static double WidenDouble(double value)
        {
            return value + 0.5;
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

using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading;

namespace SCL
{

    /// <summary>
    /// Wraps an unmanaged array of types using an IntPtr and pointer arithmetic. This overlays existing memory and
    /// provides boundaries checks, however it neither allocates nor frees the array.
    /// </summary>
    /// <typeparam name="T">the type to wrap</typeparam>
    public struct UnmanagedArray<T>
    {

        public int Size { get; }

        public IntPtr Ptr { get; }

        public UnmanagedArray(IntPtr ptr, int size)
        {
            Ptr = ptr;
            Size = size;
        }

        public IntPtr AddressOf(int index)
        {
            if (index < 0 || index >= Size) throw new IndexOutOfRangeException();
            return IntPtr.Add(Ptr, index * Marshal.SizeOf<T>());
        }

        public T this[int index]
        {
            get
            {
                var addr = AddressOf(index);
                return Marshal.PtrToStructure<T>(addr);
            }
            set
            {
                var addr = AddressOf(index);
                Marshal.StructureToPtr(value, addr, false);
            }
        }

    }

    /// <summary>
    /// Allocates native memory and maps to an array. Upon disposing, this releases the unmanaged memory.
    /// </summary>
    /// <typeparam name="T">the type to wrap</typeparam>
    public struct AutomaticUnmanagedArray<T> : IDisposable
    {

        /// <summary>
        /// The UnmanagedArray that backs this automatic unmanaged array.
        /// </summary>
        public UnmanagedArray<T> Array { get; private set; }

        /// <summary>
        /// Allocates a new instance of the AutomaticUnmanagedArray, specifying the size of the array as the number
        /// of instances in the array (not the number of bytes).
        /// </summary>
        /// <param name="size"></param>
        public AutomaticUnmanagedArray(int size)
        {
            var ptr = Marshal.AllocHGlobal(size * Marshal.SizeOf<T>());
            Array = new UnmanagedArray<T>(ptr, size);
        }

        /// <summary>
        /// Frees the underlying heap memory associated with this array. After the first call, calling this multiple
        /// times will have no effects.
        /// </summary>
        public void Dispose()
        {
            if (Array.Ptr != IntPtr.Zero) Marshal.FreeHGlobal(Array.Ptr);
            Array = new UnmanagedArray<T>();
        }

        /// <summary>
        /// Reallocates the unmanaged array to be new size. This may require a new location in memory.
        /// </summary>
        /// <param name="size"></param>
        public void Realloc(int size)
        {
            var ptr = Marshal.ReAllocHGlobal(Array.Ptr, new IntPtr(size));
            Array = new UnmanagedArray<T>(ptr, size);
        }

        /// <summary>
        /// Operator to get the element at the particular index.
        /// </summary>
        /// <param name="key">the index</param>
        public T this[int key]
        {
            get => Array[key];
            set
            {
                var array = Array;
                array[key] = value;
            }
        }

    }

    public static class Utility
    {

        /// <summary>
        /// Gets a managed array of types from a buffer callback.
        /// </summary>
        /// <param name="hint"></param>
        /// <param name="callback"></param>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static T[] CopyUnmanagedWithHint<T>(ref int hint, BufferCallback callback)
        {
            var newarraysize = hint; 
            var oldhintvalue = newarraysize;
            using (var unmanaged = new AutomaticUnmanagedArray<T>(newarraysize))
            {
                do
                {
                    newarraysize = callback(unmanaged.Array.Ptr, newarraysize);
                    if (newarraysize > unmanaged.Array.Size) unmanaged.Realloc(newarraysize);
                } while (unmanaged.Array.Size < newarraysize);

                Interlocked.CompareExchange(ref hint, newarraysize, oldhintvalue);
                
                var managed = new T[newarraysize];
                for (var i = 0; i < newarraysize; i++) managed[i] = unmanaged[i]; 
                return managed;

            }
        } 
    }

    class UnmanagedHandles<TManaged> where TManaged : class
    {
        private System.Collections.Concurrent.ConcurrentQueue<IntPtr> _handles = new System.Collections.Concurrent.ConcurrentQueue<IntPtr>();
        private TManaged[] _managed;
        public UnmanagedHandles(int maxsize = 256)
        {
            _managed = new TManaged[maxsize];
            for (var i = 1; i < maxsize+1; i++)
            {
                this._handles.Enqueue(new IntPtr(i));
            }
        }

        public void Add(TManaged managed, out IntPtr handle)
        {
            if (!_handles.TryDequeue(out handle))
            {
                throw new Exception($"You have used up more than {_managed.Length} handles! This is not supported!");
            }
            _managed[handle.ToInt32()-1] = managed;
        }

        public TManaged Get(IntPtr handle)
        {
            return _managed[handle.ToInt32() - 1];
        }

        public void Remove(IntPtr handle)
        {
            _managed[handle.ToInt32() - 1] = null;
            _handles.Enqueue(handle);
        }
    }

}

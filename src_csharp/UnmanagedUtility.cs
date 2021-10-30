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
        /// Allocates a new instance of the AutomaticUnmanagedArray, specifying the size of hte array as the number
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
            var ptr =  Marshal.ReAllocHGlobal(Array.Ptr, new IntPtr(size));
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
        public static T[] CopyUnmanagedWithHint<T>(ThreadLocal<int> hint, BufferCallback callback)
        {
            using (var unmanaged = new AutomaticUnmanagedArray<T>(hint.Value))
            {

                int actual = hint.Value;

                do
                {
                    actual = callback(unmanaged.Array.Ptr, actual);
                    if (actual > unmanaged.Array.Size) unmanaged.Realloc(actual);
                } while (unmanaged.Array.Size < actual);

                hint.Value = actual;

                var managed = new T[actual];
                for (var i = 0; i < actual; i++) managed[i] = unmanaged[i];

                return managed;

            }
        }
        
    }

    /// <summary>
    /// A thread-save way to pass data through native code and reference back to an original object. This type employs
    /// a List of objects indexed by an integer wrapped in an instance of IntPtr.
    ///
    /// Mutations to this instance use copy-on-write semantics to control concurrency. Using this method avoids GC
    /// pinning.
    /// </summary>
    /// <typeparam name="TManaged"></typeparam>
    class UnmanagedHandles<TManaged> where TManaged : class
    {
        
        private List<TManaged> _managed;

        public void Add(TManaged managed, out IntPtr handle)
        {

            int result;
            List<TManaged> update;
            List<TManaged> current;

            do
            {

                current = _managed;
                Interlocked.MemoryBarrier();

                if (current == null)
                {
                    update = new List<TManaged> {managed};
                    result = 0;
                }
                else
                {
                    
                    update = new List<TManaged>(current);
                    var index = update.FindIndex(t => t == null);

                    if (index < 0)
                    {
                        update.Add(managed);
                        result = update.Count - 1;
                    }
                    else
                    {
                        update = current;
                        update[result = index] = managed;
                    }

                }
            } while (current != Interlocked.CompareExchange(ref _managed, update, current));

            handle = new IntPtr(result + 1);

        }

        public TManaged Get(IntPtr handle)
        {
            return _managed[handle.ToInt32() - 1];
        }

        public TManaged Remove(ref IntPtr handle)
        {
            
            TManaged result;
            List<TManaged> update;
            List<TManaged> current;

            do
            {

                current = _managed;
                Interlocked.MemoryBarrier();

                if (current == null)
                    throw new InvalidOperationException("Object not present.");

                update = new List<TManaged>(current);

                var index = handle.ToInt32() - 1;
                var value = update[handle.ToInt32()];

                if (value == null) 
                    throw new InvalidOperationException($"No object at: {index})");

                result = value;
                update[index] = null;

            } while (current != Interlocked.CompareExchange(ref _managed, update, current));

            handle = IntPtr.Zero;
            return result;

        }

    }
    
}

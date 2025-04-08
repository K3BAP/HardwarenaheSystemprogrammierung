#### NVidia OpenCL ####

data_file <- "../results/nvidia/opencl/nvidia_opencl_matmul.txt"
data <- as.numeric(readLines(data_file))
str(data)

summary(data)

# Create a histogram
png("nvidia/opencl/histogram_matmul.png")
hist(data,
     main = "NVidia: OpenCL Matrix Multiplication",
     xlab = "Runtime",
     col = "skyblue", 
     border = "black")
dev.off()

# Create a box plot
png("nvidia/opencl/boxplot_matmul.png")
boxplot(data,
        main = "NVidia: OpenCL Matrix Multiplication",
        ylab = "Runtime",
        col = "lightgreen")
dev.off()

data_file <- "../results/nvidia/opencl/nvidia_opencl_copy.txt"
data <- as.numeric(readLines(data_file))
str(data)

summary(data)

# Create a histogram
png("nvidia/opencl/histogram_copy.png")
hist(data,
     main = "NVidia: OpenCL Copy Kernel",
     xlab = "Runtime",
     col = "skyblue", 
     border = "black")
dev.off()

# Create a box plot
png("nvidia/opencl/boxplot_copy.png")
boxplot(data,
        main = "NVidia: OpenCL Copy Kernel",
        ylab = "Runtime",
        col = "lightgreen")
dev.off()

#### NVidia CUDA ####
data_file <- "../results/nvidia/cuda/nvidia_cuda_matmul.txt"
data <- as.numeric(readLines(data_file))
str(data)

summary(data)

# Create a histogram
png("nvidia/cuda/histogram_matmul.png")
hist(data,
     main = "NVidia: CUDA Matrix Multiplication",
     xlab = "Runtime",
     col = "skyblue", 
     border = "black")
dev.off()

# Create a box plot
png("nvidia/cuda/boxplot_matmul.png")
boxplot(data,
        main = "NVidia: CUDA Matrix Multiplication",
        ylab = "Runtime",
        col = "lightgreen")
dev.off()

data_file <- "../results/nvidia/cuda/nvidia_cuda_copy.txt"
data <- as.numeric(readLines(data_file))
str(data)

summary(data)

# Create a histogram
png("nvidia/cuda/histogram_copy.png")
hist(data,
     main = "NVidia: CUDA Copy Kernel",
     xlab = "Runtime",
     col = "skyblue", 
     border = "black")
dev.off()

# Create a box plot
png("nvidia/cuda/boxplot_copy.png")
boxplot(data,
        main = "NVidia: CUDA Copy Kernel",
        ylab = "Runtime",
        col = "lightgreen")
dev.off()


# TODO

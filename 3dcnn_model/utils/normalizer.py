import torch

class Normalizer:
    def __init__(self):
        self.mean = None #[1 ,4, 1] per-channel stats
        self.std = None

    def fit(self, data):
        """
        Calculate and store the mean and standard deviation for each channel.
        :param data: Tensor of shape [B, C=4, L=66]
        """
        if not torch.is_tensor(data):
            raise TypeError("Input data must be a PyTorch tensor.")

        # Calculate mean and std across batch and length dimensions (B and L)
        self.mean = torch.mean(data, dim=(0, 2), keepdim=True)  # Shape [1, C, 1]
        self.std = torch.std(data, dim=(0, 2), keepdim=True)  # Shape [1, C, 1]

    def transform(self, data, channel=0):
        """
        Apply normalization using the stored mean and std.
        :param data: Tensor of shape [B, C=4, L=66]
        :return: Normalized data
        """
        if self.mean is None or self.std is None:
            raise ValueError("Normalizer has not been fitted with data.")
        if not torch.is_tensor(data):
            raise TypeError("Input data must be a PyTorch tensor.")
        
        # Move mean and std to the same device as data if they are not already on the same device
        mean = self.mean.to(data.device)
        std = self.std.to(data.device)

        # Normalize data (element-wise)
        normalized_data = (data - mean[:,channel:,:]) / (std[:,channel:,:] + 1e-6)  # Add epsilon to avoid division by zero
        return normalized_data

    def fit_transform(self, data):
        """
        Fit and transform the data in one step.
        :param data: Tensor of shape [B, C=4, L=66]
        :return: Normalized data
        """
        self.fit(data)
        return self.transform(data)

    def inverse_transform(self, data, channel=0):
        """
        Reverse the normalization process to recover the original scale.
        :param data: Normalized tensor of shape [B, C=4, L=66]
        :return: Original data before normalization
        """
        if self.mean is None or self.std is None:
            raise ValueError("Normalizer has not been fitted with data.")
        if not torch.is_tensor(data):
            raise TypeError("Input data must be a PyTorch tensor.")

        # Move mean and std to the same device as data if they are not already on the same device
        mean = self.mean.to(data.device)
        std = self.std.to(data.device)

        # Reverse normalization (element-wise)
        original_data = data * (std[:,channel:,:] + 1e-6) + mean[:,channel:,:]
        return original_data

    def save(self, filepath):
        """
        Save the normalizer's mean and std to a file.
        :param filepath: Path to save the normalizer parameters.
        """
        torch.save({'mean': self.mean, 'std': self.std}, filepath)

    def load(self, filepath):
        """
        Load the normalizer's mean and std from a file.
        :param filepath: Path to load the normalizer parameters from.
        """
        checkpoint = torch.load(filepath)
        self.mean = checkpoint['mean']
        self.std = checkpoint['std']

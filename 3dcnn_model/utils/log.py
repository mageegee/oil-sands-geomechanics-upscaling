class Logger:
    def __init__(self, save_path):
        self.save_path = save_path
    
    def log(self, epoch, phase, losses):
        # Format the loss into a string
        log_str = f"epoch: {epoch} - {phase} Loss: {[round(i,8) for i in losses]}"
        
        with open(self.save_path, 'a') as file:
            file.write(log_str + '\n')

            if phase == "Val":
                file.write("#" * 10 + "\n")
